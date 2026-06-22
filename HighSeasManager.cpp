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
