/**
 * @file HighSeasManager.h
 * @brief 公海池自动回收管理器声明。
 *
 * 研究定位：本文件属于业务服务层，用于表达客户超过指定天数未跟进后自动释放到
 * 公海池的规则。它独立于主窗口存在，由 main.cpp 在登录后启动。
 *
 * 主要职责：声明定时检查、启动停止、逾期天数配置以及客户被回收后的通知信号。
 */
#ifndef HIGHSEASMANAGER_H
#define HIGHSEASMANAGER_H

#include "ICustomerRepository.h"

#include <QObject>
#include <QTimer>
#include <memory>

class HighSeasManager : public QObject
{
    Q_OBJECT

public:
    explicit HighSeasManager(std::shared_ptr<ICustomerRepository> repository,
                             int overdueDays = 30,
                             QObject* parent = nullptr);

    void setOverdueDays(int days);
    int overdueDays() const;

public slots:
    void start(int intervalMs = 60000);
    void stop();
    void checkNow();

signals:
    void customerRecycled(const QString& customerId, const QString& previousOwnerId);
    void errorOccurred(const QString& message);

private:
    std::shared_ptr<ICustomerRepository> m_repository;
    QTimer m_timer;
    int m_overdueDays;
};

#endif // HIGHSEASMANAGER_H
