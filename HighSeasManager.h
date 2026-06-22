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
