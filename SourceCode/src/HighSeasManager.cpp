/**
 * @file HighSeasManager.cpp
 * @brief 公海池自动回收管理器实现。
 *
 * 研究定位：本文件实现客户逾期未跟进后的自动回收机制，是客户资产流转规则的后台
 * 执行者。它通过 QTimer 周期运行，不直接参与界面渲染。
 *
 * 主要职责：定时扫描所有客户，判断最后跟进时间是否超过阈值；若客户仍有负责人，
 * 则调用仓库释放到公海池，并发出回收结果信号。
 */
#include "HighSeasManager.h"

#include <QDateTime>
#include <QtGlobal>

#include <utility>

HighSeasManager::HighSeasManager(std::shared_ptr<ICustomerRepository> repository,
                                 int overdueDays,
                                 QObject* parent)
    : QObject(parent)
    , m_repository(std::move(repository))
    , m_overdueDays(overdueDays)
{
    connect(&m_timer, &QTimer::timeout, this, &HighSeasManager::checkNow);
}

void HighSeasManager::setOverdueDays(int days)
{
    m_overdueDays = qMax(1, days);
}

int HighSeasManager::overdueDays() const
{
    return m_overdueDays;
}

void HighSeasManager::start(int intervalMs)
{
    m_timer.start(qMax(1000, intervalMs));
    checkNow();
}

void HighSeasManager::stop()
{
    m_timer.stop();
}

void HighSeasManager::checkNow()
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    for (auto customer : m_repository->getAllCustomers()) {
        const QString previousOwner = customer.getOwnerId();
        if (previousOwner.isEmpty() || !customer.getLastFollowTime().isValid()) {
            continue;
        }

        if (customer.getLastFollowTime().daysTo(now) < m_overdueDays) {
            continue;
        }

        customer.setOwnerId(QString());
        if (!m_repository->saveCustomer(customer)) {
            emit errorOccurred(QStringLiteral("Failed to recycle overdue customer: %1").arg(customer.getId()));
            continue;
        }

        FollowRecord audit(
            QStringLiteral("AUTO_SEA_%1").arg(QDateTime::currentMSecsSinceEpoch()),
            customer.getId(),
            QStringLiteral("system"),
            QStringLiteral("HighSeasManager"),
            QStringLiteral("Customer recycled automatically because it is overdue."),
            now
        );
        m_repository->insertFollowRecord(audit);
        emit customerRecycled(customer.getId(), previousOwner);
    }
}
