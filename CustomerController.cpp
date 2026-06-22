#include "CustomerController.h"

#include <utility>

CustomerController::CustomerController(std::shared_ptr<ICustomerRepository> repository,
                                       const User& currentUser,
                                       QObject* parent)
    : QObject(parent)
    , m_repository(std::move(repository))
    , m_currentUser(currentUser)
{
}

const User& CustomerController::currentUser() const
{
    return m_currentUser;
}

void CustomerController::loadCustomers()
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }

    std::vector<Customer> customers;
    switch (m_currentUser.getRole()) {
    case UserRole::Sales:
        customers = m_repository->getCustomersBySales(m_currentUser.getUserId());
        break;
    case UserRole::Manager:
        customers = m_repository->getCustomersByDepartment(m_currentUser.getDepartment());
        break;
    case UserRole::Admin:
        customers = m_repository->getAllCustomers();
        break;
    }

    emit customersLoaded(customers);
}

void CustomerController::searchCustomers(const QString& keyword)
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }

    const QString key = keyword.trimmed();
    if (key.isEmpty()) {
        loadCustomers();
        return;
    }

    emit customersLoaded(m_repository->searchCustomers(key, m_currentUser));
}

void CustomerController::saveCustomer(const Customer& customer)
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }
    if (!customer.isValid()) {
        emit errorOccurred(QStringLiteral("Customer name or phone number is invalid."));
        return;
    }
    if (!m_repository->saveCustomer(customer)) {
        emit errorOccurred(QStringLiteral("Failed to save customer."));
        return;
    }

    emit customerSaved(customer);
    loadCustomers();
}

void CustomerController::deleteCustomer(const QString& customerId)
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }
    if (!m_repository->deleteCustomer(customerId)) {
        emit errorOccurred(QStringLiteral("Failed to delete customer."));
        return;
    }

    emit customerDeleted(customerId);
    loadCustomers();
}

void CustomerController::loadFollowRecords(const QString& customerId)
{
    if (!m_repository) {
        emit errorOccurred(QStringLiteral("Repository is not available."));
        return;
    }

    emit followRecordsLoaded(customerId, m_repository->getFollowRecords(customerId));
}
