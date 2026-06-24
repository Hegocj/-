/**
 * @file ICustomerRepository.h
 * @brief 客户信息管理系统的数据仓库接口。
 *
 * 研究定位：本文件属于数据访问抽象层，定义界面层和业务服务层访问持久化数据时
 * 依赖的统一契约。通过接口隔离，系统可以在不改变界面代码的前提下替换底层存储实现。
 *
 * 主要职责：声明数据库初始化、登录认证、员工管理、客户增删改查、跟进记录、公海池
 * 流转和客户资产交接等核心数据操作。SQLiteCustomerRepo 是该接口的当前实现类。
 */
#ifndef ICUSTOMERREPOSITORY_H
#define ICUSTOMERREPOSITORY_H

#include <vector>
#include <QString>
#include "GlobalModels.h"
#include "RandomDataGenerator.h"

/**
 * @brief CRM 系统数据仓库接口定义
 * 该接口作为"系统骨架"，用于解耦业务逻辑层与底层数据库实现
 */
class ICustomerRepository {
public:
    virtual ~ICustomerRepository() = default;

    // ==========================================
    // 0. 数据库初始化与生命周期管理
    // ==========================================

    /**
     * @brief 初始化本地 SQLite 数据库
     * @param dbPath 本地数据库文件路径（如 "crm.db"）
     * @return 初始化成功返回 true
     * @note 若检测到文件不存在，应自动创建 user, customer, follow_record 三张表并写入初始管理员账号
     */
    virtual bool initializeDatabase(const QString& dbPath) = 0;

    /**
     * @brief 按数量生成本地演示数据
     * @param config 指定客户、销售、经理、管理员数量
     * @param clearExisting 是否先清空旧的用户、客户和跟进记录
     * @return 生成并写入成功返回 true
     */
    virtual bool seedRandomData(const RandomDataConfig& config, bool clearExisting = true) = 0;

    /**
     * @brief 关闭数据库连接，释放资源
     */
    virtual void closeDatabase() = 0;


    // ==========================================
    // 1. 身份认证与员工/用户管理
    // ==========================================

    /**
     * @brief 验证用户登录
     * @param username 用户名
     * @param password 密码
     * @param outUser 传引用：登录成功时返回完整的 User 对象（内含角色标签）
     * @return 登录成功返回 true
     */
    virtual bool checkLogin(const QString& username, const QString& password, User& outUser) = 0;

    /**
     * @brief 获取系统内所有的员工列表
     * @return 员工对象数组
     * @note 用于管理员界面下拉菜单的数据填充，如选择交接人、选择分配团队的销售等
     */
    virtual std::vector<User> getAllUsers() = 0;
    virtual bool saveUser(const User& user, const QString& password = QString()) = 0;

    /**
     * @brief 修改员工信息/权限
     * @param user 包含新权限标签(UserRole)或新部门的 User 对象
     * @return 是否修改成功（内部执行 UPDATE user 表）
     */
    virtual bool updateUser(const User& user) = 0;
    virtual bool updateUserPassword(const QString& userId, const QString& password) = 0;
    virtual bool deleteUser(const QString& userId) = 0;


    // ==========================================
    // 2. 客户数据核心增删改查（CRUD）
    // ==========================================

    /**
     * @brief 管理员/老板接口：查询全盘所有客户
     */
    virtual std::vector<Customer> getAllCustomers() = 0;

    /**
     * @brief 销售专属接口：获取某个特定销售负责的客户列表
     * @param salesId 销售员工的ID（工号）
     */
    virtual std::vector<Customer> getCustomersBySales(const QString& salesId) = 0;

    /**
     * @brief 经理专属接口：获取某个部门/团队下的所有客户列表
     * @param department 部门名称
     */
    virtual std::vector<Customer> getCustomersByDepartment(const QString& department) = 0;

    /**
     * @brief 关键字模糊搜索（姓名或电话）
     * @param keyword 搜索关键词
     * @param currentUser 当前操作的用户（用于后端自动拼接 SQLite 的 WHERE 条件，实现权限越界拦截）
     */
    virtual std::vector<Customer> searchCustomers(const QString& keyword, const User& currentUser) = 0;

    /**
     * @brief 新增或更新客户信息
     * @param customer 客户对象类
     * @return 是否保存成功（内部执行 INSERT OR REPLACE 语句）
     */
    virtual bool saveCustomer(const Customer& customer) = 0;

    /**
     * @brief 删除客户
     */
    virtual bool deleteCustomer(const QString& customerId) = 0;


    // ==========================================
    // 3. 动态时间线核心接口
    // ==========================================

    /**
     * @brief 获取某个客户的所有历史跟进记录
     * @param customerId 客户ID
     * @return 跟进记录类数组（按时间倒序 `ORDER BY date DESC`）
     */
    virtual std::vector<FollowRecord> getFollowRecords(const QString& customerId) = 0;

    /**
     * @brief 提交一条新的跟进记录
     * @param record 包含操作人ID、名字、客户ID、内容的记录类对象
     */
    virtual bool insertFollowRecord(const FollowRecord& record) = 0;


    // ==========================================
    // 4. 高级商业业务接口（资产交接与公海池）
    // ==========================================

    /**
     * @brief 管理员专属：分配销售到指定团队
     * @param salesId 销售工号
     * @param targetDepartment 目标部门名称（如 "华南销售部"）
     */
    virtual bool assignSalesToDepartment(const QString& salesId, const QString& targetDepartment) = 0;

    /**
     * @brief 员工离职/调岗，资产一键无缝交接
     * @param fromSalesId 移交（离职）销售的ID
     * @param toSalesId 接收（接手）销售的ID
     * @return 是否交接成功（内部执行一条 UPDATE 语句批量涂改客户的 owner_id）
     */
    virtual bool transferCustomers(const QString& fromSalesId, const QString& toSalesId) = 0;

    /**
     * @brief 获取公海池中所有未被认领的客户列表
     * @return 客户对象数组（内部执行 SELECT * FROM customer WHERE owner_id IS NULL）
     */
    virtual std::vector<Customer> getHighSeasCustomers() = 0;

    /**
     * @brief 认领公海池客户
     * @param customerId 想要认领的客户ID
     * @param salesId 认领人的销售ID
     */
    virtual bool claimCustomer(const QString& customerId, const QString& salesId) = 0;

    /**
     * @brief 释放客户到公海池
     * @param customerId 客户ID
     * @param salesId 释放该客户的销售ID（用于权限校验，确保只有主人能操作）
     * @return 是否释放成功
     */
    virtual bool releaseCustomerToHighSeas(const QString& customerId, const QString& salesId) = 0;
};

#endif // ICUSTOMERREPOSITORY_H
