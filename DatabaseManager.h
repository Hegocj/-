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
