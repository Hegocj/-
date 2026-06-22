#ifndef CUSTOMERCONTROLLER_H
#define CUSTOMERCONTROLLER_H

#include "ICustomerRepository.h"

#include <QObject>
#include <memory>
#include <vector>

class CustomerController : public QObject
{
    Q_OBJECT

public:
    explicit CustomerController(std::shared_ptr<ICustomerRepository> repository,
                                const User& currentUser,
                                QObject* parent = nullptr);

    const User& currentUser() const;

public slots:
    void loadCustomers();
    void searchCustomers(const QString& keyword);
    void saveCustomer(const Customer& customer);
    void deleteCustomer(const QString& customerId);
    void loadFollowRecords(const QString& customerId);

signals:
    void customersLoaded(const std::vector<Customer>& customers);
    void followRecordsLoaded(const QString& customerId, const std::vector<FollowRecord>& records);
    void customerSaved(const Customer& customer);
    void customerDeleted(const QString& customerId);
    void errorOccurred(const QString& message);

private:
    std::shared_ptr<ICustomerRepository> m_repository;
    User m_currentUser;
};

#endif // CUSTOMERCONTROLLER_H
