/**
 * @file DatabaseManager.h
 * @brief SQLite 数据库连接管理器声明。
 *
 * 研究定位：本文件属于数据访问基础设施层，用单例方式统一管理 Qt SQL 连接。它为
 * SQLiteCustomerRepo 提供数据库打开、关闭和事务控制能力。
 *
 * 主要职责：声明数据库连接生命周期方法、事务开始/提交/回滚方法和连接状态查询。
 * 通过集中管理连接名和 QSqlDatabase 对象，降低多处打开数据库造成的资源混乱风险。
 */
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool open(const QString& dbPath, const QString& connectionName = QStringLiteral("crm_connection"));
    void close();

    QSqlDatabase database() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isOpen() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif // DATABASEMANAGER_H
