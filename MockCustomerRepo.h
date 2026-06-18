#ifndef MOCKCUSTOMERREPO_H
#define MOCKCUSTOMERREPO_H

#include "ICustomerRepository.h"
#include <vector>

class MockCustomerRepo : public ICustomerRepository {
public:
    // 构造函数预设几条假数据
    MockCustomerRepo() {
        m_users.push_back(User("admin", "管理员", "总部", UserRole::Admin));
        m_users.push_back(User("sales01", "小张", "销售部", UserRole::Sales));
        m_users.push_back(User("mgr_001", "销售经理老王", "销售部", UserRole::Manager));

        m_customers.push_back(Customer("C001", "张三", "13800138000", "VIP", QDateTime::currentDateTime(), "sales01"));
        m_customers.push_back(Customer("C002", "李四", "13900139000", "普通", QDateTime::currentDateTime(), "")); // 公海客户
    }

    bool initializeDatabase(const QString& dbPath) override { return true; }
    void closeDatabase() override {}

    bool checkLogin(const QString& username, const QString& password, User& outUser) override {
        // 简易明文多角色校验与实体完整回填
        if (username == "admin" && password == "123") {
            outUser = User("admin", "管理员", "总部", UserRole::Admin);
            return true;
        }
        else if (username == "sales01" && password == "123") {
            outUser = User("sales01", "销售小张", "销售部", UserRole::Sales);
            return true;
        }
        else if (username == "manager01" && password == "123") {
            outUser = User("mgr_001", "销售经理老王", "销售部", UserRole::Manager);
            return true;
        }
        return false;
    }

    std::vector<User> getAllUsers() override { return m_users; }

    bool updateUser(const User& user) override { return true; }

    std::vector<Customer> getAllCustomers() override { return m_customers; }

    std::vector<Customer> getCustomersBySales(const QString& salesId) override {
        std::vector<Customer> res;
        //qDebug()<<"寻找对应客户开始";
        for (const auto& c : m_customers) {
            //debug
            qDebug()<<c.getOwnerId();
            if (c.getOwnerId() == salesId) {
                res.push_back(c);
                //qDebug()<<c.getOwnerId();
            }
        }
        return res;
    }

    std::vector<Customer> getCustomersByDepartment(const QString& department) override {
        return m_customers; // 简化版直接返回全部
    }

    std::vector<Customer> searchCustomers(const QString& keyword, const User& currentUser) override {
        return m_customers; // 简化版直接返回全部
    }

    bool saveCustomer(const Customer& customer) override {
        m_customers.push_back(customer);
        return true;
    }

    bool deleteCustomer(const QString& customerId) override { return true; }

    std::vector<FollowRecord> getFollowRecords(const QString& customerId) override {
            return {
                FollowRecord(
                    "0001",
                    "C001",
                    "sales01",
                    "小张",
                    "客户初次电话回访",
                    QDateTime::fromString("2026-06-17", "yyyy-MM-dd")
                    )
            }; //简单初始化
    }

    bool insertFollowRecord(const FollowRecord& record) override { return true; }

    bool assignSalesToDepartment(const QString& salesId, const QString& targetDepartment) override { return true; }

    bool transferCustomers(const QString& fromSalesId, const QString& toSalesId) override { return true; }

    std::vector<Customer> getHighSeasCustomers() override {
        std::vector<Customer> res;
        for (const auto& c : m_customers) {
            if (c.getOwnerId().isEmpty()) res.push_back(c);
        }
        return res;
    }

    bool claimCustomer(const QString& customerId, const QString& salesId) override { return true; }

    bool releaseCustomerToHighSeas(const QString& customerId, const QString& salesId) override { return true; }

private:
    std::vector<User> m_users;
    std::vector<Customer> m_customers;
};

#endif // MOCKCUSTOMERREPO_H