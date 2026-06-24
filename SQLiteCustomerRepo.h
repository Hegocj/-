#ifndef SQLITECUSTOMERREPO_H
#define SQLITECUSTOMERREPO_H

#include "ICustomerRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class SQLiteCustomerRepo : public ICustomerRepository {
public:
    SQLiteCustomerRepo();
    ~SQLiteCustomerRepo() override;

    // 数据库生命周期
    bool initializeDatabase(const QString& dbPath) override;
    void closeDatabase() override;

    // 身份认证与用户管理
    bool checkLogin(const QString& username, const QString& password, User& outUser) override;
    std::vector<User> getAllUsers() override;
    bool saveUser(const User& user, const QString& password = QString()) override;
    bool updateUser(const User& user) override;
    bool updateUserPassword(const QString& userId, const QString& password) override;
    bool deleteUser(const QString& userId) override;

    // 客户数据 CRUD
    std::vector<Customer> getAllCustomers() override;
    std::vector<Customer> getCustomersBySales(const QString& salesId) override;
    std::vector<Customer> getCustomersByDepartment(const QString& department) override;
    std::vector<Customer> searchCustomers(const QString& keyword, const User& currentUser) override;
    bool saveCustomer(const Customer& customer) override;
    bool deleteCustomer(const QString& customerId) override;

    // 跟进记录
    std::vector<FollowRecord> getFollowRecords(const QString& customerId) override;
    bool insertFollowRecord(const FollowRecord& record) override;

    // 高级业务接口
    bool assignSalesToDepartment(const QString& salesId, const QString& targetDepartment) override;
    bool transferCustomers(const QString& fromSalesId, const QString& toSalesId) override;
    std::vector<Customer> getHighSeasCustomers() override;
    bool claimCustomer(const QString& customerId, const QString& salesId) override;
    bool releaseCustomerToHighSeas(const QString& customerId, const QString& salesId) override;

private:
    bool createTables();
    bool insertInitialData();
    User userFromQuery(const QSqlQuery& query);
    Customer customerFromQuery(const QSqlQuery& query);
    FollowRecord followRecordFromQuery(const QSqlQuery& query);

    QSqlDatabase m_db;
    bool m_initialized;
};

#endif // SQLITECUSTOMERREPO_H
