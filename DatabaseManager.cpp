#include "DatabaseManager.h"

#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString& dbPath, const QString& connectionName)
{
    if (m_db.isOpen() && m_db.databaseName() == dbPath) {
        return true;
    }

    close();

    m_connectionName = connectionName;
    if (QSqlDatabase::contains(m_connectionName)) {
        m_db = QSqlDatabase::database(m_connectionName);
    } else {
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    }

    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "Open database failed:" << m_db.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    const QString connectionName = m_connectionName;
    if (m_db.isValid()) {
        if (m_db.isOpen()) {
            m_db.close();
        }
        m_db = QSqlDatabase();
    }

    if (!connectionName.isEmpty() && QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    m_connectionName.clear();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

bool DatabaseManager::beginTransaction()
{
    return m_db.isOpen() && m_db.transaction();
}

bool DatabaseManager::commitTransaction()
{
    return m_db.isOpen() && m_db.commit();
}

bool DatabaseManager::rollbackTransaction()
{
    return m_db.isOpen() && m_db.rollback();
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}
