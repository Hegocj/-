#include "SQLiteCustomerRepo.h"
#include "DatabaseManager.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>

SQLiteCustomerRepo::SQLiteCustomerRepo()
    : m_initialized(false)
{
}

SQLiteCustomerRepo::~SQLiteCustomerRepo()
{
    closeDatabase();
}

bool SQLiteCustomerRepo::initializeDatabase(const QString& dbPath)
{
    if (!DatabaseManager::instance().open(dbPath)) {
        return false;
    }

    m_db = DatabaseManager::instance().database();
    if (!createTables()) {
        return false;
    }

    if (!insertInitialData()) {
        return false;
    }

    QSqlQuery compatibilityQuery(m_db);
    compatibilityQuery.prepare("UPDATE user SET password = ?, is_active = 1 WHERE id = ? AND role = ?");
    compatibilityQuery.addBindValue(QStringLiteral("123"));
    compatibilityQuery.addBindValue(QStringLiteral("sales01"));
    compatibilityQuery.addBindValue(QStringLiteral("sales"));
    compatibilityQuery.exec();

    QSqlQuery unrecoverableHashQuery(m_db);
    unrecoverableHashQuery.prepare(QStringLiteral(
        "UPDATE user SET password = ? "
        "WHERE length(password) = 64 "
        "AND lower(password) = password "
        "AND password GLOB '[0-9a-f]*'"
    ));
    unrecoverableHashQuery.addBindValue(QStringLiteral("123"));
    unrecoverableHashQuery.exec();

    QSqlQuery defaultUserQuery(m_db);
    defaultUserQuery.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO user (id, username, department, role, password, is_active) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ));
    defaultUserQuery.addBindValue(QStringLiteral("admin"));
    defaultUserQuery.addBindValue(QStringLiteral("admin"));
    defaultUserQuery.addBindValue(QStringLiteral("\u603b\u90e8"));
    defaultUserQuery.addBindValue(QStringLiteral("admin"));
    defaultUserQuery.addBindValue(QStringLiteral("123"));
    defaultUserQuery.addBindValue(1);
    defaultUserQuery.exec();

    defaultUserQuery.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO user (id, username, department, role, password, is_active) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ));
    defaultUserQuery.addBindValue(QStringLiteral("sales01"));
    defaultUserQuery.addBindValue(QStringLiteral("sales01"));
    defaultUserQuery.addBindValue(QStringLiteral("\u9500\u552e\u90e8"));
    defaultUserQuery.addBindValue(QStringLiteral("sales"));
    defaultUserQuery.addBindValue(QStringLiteral("123"));
    defaultUserQuery.addBindValue(1);
    defaultUserQuery.exec();

    defaultUserQuery.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO user (id, username, department, role, password, is_active) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ));
    defaultUserQuery.addBindValue(QStringLiteral("mgr_001"));
    defaultUserQuery.addBindValue(QStringLiteral("mgr_001"));
    defaultUserQuery.addBindValue(QStringLiteral("\u9500\u552e\u90e8"));
    defaultUserQuery.addBindValue(QStringLiteral("manager"));
    defaultUserQuery.addBindValue(QStringLiteral("123"));
    defaultUserQuery.addBindValue(1);
    defaultUserQuery.exec();

    QSqlQuery managerCompatibilityQuery(m_db);
    managerCompatibilityQuery.prepare(QStringLiteral(
        "UPDATE user SET is_active = 1, role = ?, department = ? WHERE id = ?"
    ));
    managerCompatibilityQuery.addBindValue(QStringLiteral("manager"));
    managerCompatibilityQuery.addBindValue(QStringLiteral("\u9500\u552e\u90e8"));
    managerCompatibilityQuery.addBindValue(QStringLiteral("mgr_001"));
    managerCompatibilityQuery.exec();

    m_initialized = true;
    return true;
}

void SQLiteCustomerRepo::closeDatabase()
{
    m_db = QSqlDatabase();
    DatabaseManager::instance().close();
}

bool SQLiteCustomerRepo::createTables()
{
    QSqlQuery query(m_db);

    const QString createUserTable = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS user ("
        "id TEXT PRIMARY KEY,"
        "username TEXT NOT NULL,"
        "department TEXT NOT NULL,"
        "role TEXT NOT NULL,"
        "password TEXT NOT NULL,"
        "is_active INTEGER NOT NULL DEFAULT 1"
        ")"
    );

    if (!query.exec(createUserTable)) {
        qDebug() << "Create user table error:" << query.lastError();
        return false;
    }

    const QString createCustomerTable = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS customer ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "phone TEXT NOT NULL,"
        "level TEXT,"
        "last_follow_time INTEGER,"
        "owner_id TEXT"
        ")"
    );

    if (!query.exec(createCustomerTable)) {
        qDebug() << "Create customer table error:" << query.lastError();
        return false;
    }

    const QString createFollowRecordTable = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS follow_record ("
        "id TEXT PRIMARY KEY,"
        "customer_id TEXT NOT NULL,"
        "operator_id TEXT NOT NULL,"
        "operator_name TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "date INTEGER NOT NULL,"
        "FOREIGN KEY (customer_id) REFERENCES customer(id)"
        ")"
    );

    if (!query.exec(createFollowRecordTable)) {
        qDebug() << "Create follow_record table error:" << query.lastError();
        return false;
    }

    return true;
}

bool SQLiteCustomerRepo::insertInitialData()
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM user"))) {
        return false;
    }
    if (query.next() && query.value(0).toInt() > 0) {
        return true;
    }

    const QString insertUser = QStringLiteral(
        "INSERT OR REPLACE INTO user (id, username, department, role, password, is_active) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    query.prepare(insertUser);
    query.addBindValue(QStringLiteral("admin"));
    query.addBindValue(QStringLiteral("admin"));
    query.addBindValue(QStringLiteral("\u603b\u90e8"));
    query.addBindValue(QStringLiteral("admin"));
    query.addBindValue(QStringLiteral("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert admin user error:" << query.lastError();
        return false;
    }

    query.prepare(insertUser);
    query.addBindValue(QStringLiteral("sales01"));
    query.addBindValue(QStringLiteral("sales01"));
    query.addBindValue(QStringLiteral("\u9500\u552e\u90e8"));
    query.addBindValue(QStringLiteral("sales"));
    query.addBindValue(QStringLiteral("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert sales01 user error:" << query.lastError();
        return false;
    }

    query.prepare(insertUser);
    query.addBindValue(QStringLiteral("mgr_001"));
    query.addBindValue(QStringLiteral("mgr_001"));
    query.addBindValue(QStringLiteral("\u9500\u552e\u90e8"));
    query.addBindValue(QStringLiteral("manager"));
    query.addBindValue(QStringLiteral("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert mgr_001 user error:" << query.lastError();
        return false;
    }

    const QString insertCustomer = QStringLiteral(
        "INSERT OR REPLACE INTO customer (id, name, phone, level, last_follow_time, owner_id) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    query.prepare(insertCustomer);
    query.addBindValue(QStringLiteral("C001"));
    query.addBindValue(QStringLiteral("\u5f20\u4e09"));
    query.addBindValue(QStringLiteral("13800138000"));
    query.addBindValue(QStringLiteral("VIP"));
    query.addBindValue(QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.addBindValue(QStringLiteral("sales01"));
    if (!query.exec()) {
        qDebug() << "Insert C001 customer error:" << query.lastError();
        return false;
    }

    query.prepare(insertCustomer);
    query.addBindValue(QStringLiteral("C002"));
    query.addBindValue(QStringLiteral("\u674e\u56db"));
    query.addBindValue(QStringLiteral("13900139000"));
    query.addBindValue(QStringLiteral("\u666e\u901a"));
    query.addBindValue(QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.addBindValue(QString());
    if (!query.exec()) {
        qDebug() << "Insert C002 customer error:" << query.lastError();
        return false;
    }

    const QString insertFollow = QStringLiteral(
        "INSERT OR REPLACE INTO follow_record (id, customer_id, operator_id, operator_name, content, date) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    query.prepare(insertFollow);
    query.addBindValue(QStringLiteral("FR001"));
    query.addBindValue(QStringLiteral("C001"));
    query.addBindValue(QStringLiteral("sales01"));
    query.addBindValue(QStringLiteral("sales01"));
    query.addBindValue(QStringLiteral("\u5ba2\u6237\u9996\u6b21\u7535\u8bdd\u56de\u8bbf"));
    query.addBindValue(QDateTime::fromString(QStringLiteral("2026-06-17"), QStringLiteral("yyyy-MM-dd")).toMSecsSinceEpoch());
    if (!query.exec()) {
        qDebug() << "Insert follow record error:" << query.lastError();
        return false;
    }

    return true;
}

User SQLiteCustomerRepo::userFromQuery(const QSqlQuery& query)
{
    User user;
    user.setUserId(query.value(QStringLiteral("id")).toString());
    user.setUsername(query.value(QStringLiteral("username")).toString());
    user.setDepartment(query.value(QStringLiteral("department")).toString());
    user.setPassword(query.value(QStringLiteral("password")).toString());

    const QString roleStr = query.value(QStringLiteral("role")).toString();
    if (roleStr == QStringLiteral("admin")) {
        user.setRole(UserRole::Admin);
    } else if (roleStr == QStringLiteral("manager")) {
        user.setRole(UserRole::Manager);
    } else {
        user.setRole(UserRole::Sales);
    }

    user.setActive(query.value(QStringLiteral("is_active")).toBool());
    return user;
}

Customer SQLiteCustomerRepo::customerFromQuery(const QSqlQuery& query)
{
    Customer customer;
    customer.setId(query.value(QStringLiteral("id")).toString());
    customer.setName(query.value(QStringLiteral("name")).toString());
    customer.setPhone(query.value(QStringLiteral("phone")).toString());
    const QString rawLevel = query.value(QStringLiteral("level")).toString().trimmed();
    customer.setLevel(rawLevel.compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0
                          ? QStringLiteral("VIP")
                          : QStringLiteral("\u666e\u901a"));
    customer.setLastFollowTime(QDateTime::fromMSecsSinceEpoch(query.value(QStringLiteral("last_follow_time")).toLongLong()));
    customer.setOwnerId(query.value(QStringLiteral("owner_id")).toString());
    return customer;
}

FollowRecord SQLiteCustomerRepo::followRecordFromQuery(const QSqlQuery& query)
{
    FollowRecord record;
    record.setRecordId(query.value(QStringLiteral("id")).toString());
    record.setCustomerId(query.value(QStringLiteral("customer_id")).toString());
    record.setOperatorId(query.value(QStringLiteral("operator_id")).toString());
    record.setOperatorName(query.value(QStringLiteral("operator_name")).toString());
    record.setContent(query.value(QStringLiteral("content")).toString());
    record.setDate(QDateTime::fromMSecsSinceEpoch(query.value(QStringLiteral("date")).toLongLong()));
    return record;
}

bool SQLiteCustomerRepo::checkLogin(const QString& username, const QString& password, User& outUser)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT * FROM user WHERE username = ? AND password = ? AND is_active = 1"));
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        outUser = userFromQuery(query);
        return true;
    }

    return false;
}

std::vector<User> SQLiteCustomerRepo::getAllUsers()
{
    std::vector<User> users;
    QSqlQuery query(QStringLiteral("SELECT * FROM user"), m_db);
    while (query.next()) {
        users.push_back(userFromQuery(query));
    }
    return users;
}

bool SQLiteCustomerRepo::updateUser(const User& user)
{
    return saveUser(user);
}

bool SQLiteCustomerRepo::saveUser(const User& user, const QString& password)
{
    QString roleStr = QStringLiteral("sales");
    if (user.getRole() == UserRole::Admin) {
        roleStr = QStringLiteral("admin");
    } else if (user.getRole() == UserRole::Manager) {
        roleStr = QStringLiteral("manager");
    }

    QSqlQuery existsQuery(m_db);
    existsQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM user WHERE id = ?"));
    existsQuery.addBindValue(user.getUserId());
    if (!existsQuery.exec() || !existsQuery.next()) {
        return false;
    }

    const bool exists = existsQuery.value(0).toInt() > 0;
    QSqlQuery query(m_db);
    if (exists && password.isEmpty()) {
        query.prepare(QStringLiteral(
            "UPDATE user SET username = ?, department = ?, role = ?, is_active = ? WHERE id = ?"
        ));
        query.addBindValue(user.getUsername());
        query.addBindValue(user.getDepartment());
        query.addBindValue(roleStr);
        query.addBindValue(user.isActive() ? 1 : 0);
        query.addBindValue(user.getUserId());
        return query.exec();
    }

    if (exists) {
        query.prepare(QStringLiteral(
            "UPDATE user SET username = ?, department = ?, role = ?, is_active = ?, password = ? WHERE id = ?"
        ));
        query.addBindValue(user.getUsername());
        query.addBindValue(user.getDepartment());
        query.addBindValue(roleStr);
        query.addBindValue(user.isActive() ? 1 : 0);
        query.addBindValue(password);
        query.addBindValue(user.getUserId());
        return query.exec();
    }

    query.prepare(QStringLiteral(
        "INSERT INTO user (id, username, department, role, password, is_active) VALUES (?, ?, ?, ?, ?, ?)"
    ));
    query.addBindValue(user.getUserId());
    query.addBindValue(user.getUsername());
    query.addBindValue(user.getDepartment());
    query.addBindValue(roleStr);
    query.addBindValue(password.isEmpty() ? QStringLiteral("123") : password);
    query.addBindValue(user.isActive() ? 1 : 0);
    return query.exec();
}

bool SQLiteCustomerRepo::updateUserPassword(const QString& userId, const QString& password)
{
    if (userId.trimmed().isEmpty() || password.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE user SET password = ? WHERE id = ?"));
    query.addBindValue(password);
    query.addBindValue(userId);
    return query.exec();
}

std::vector<Customer> SQLiteCustomerRepo::getAllCustomers()
{
    std::vector<Customer> customers;
    QSqlQuery query(QStringLiteral(
        "SELECT * FROM customer "
        "ORDER BY CASE WHEN upper(level) = 'VIP' THEN 0 ELSE 1 END, last_follow_time DESC, id ASC"
    ), m_db);
    while (query.next()) {
        customers.push_back(customerFromQuery(query));
    }
    return customers;
}

std::vector<Customer> SQLiteCustomerRepo::getCustomersBySales(const QString& salesId)
{
    std::vector<Customer> customers;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT * FROM customer WHERE owner_id = ? "
        "ORDER BY CASE WHEN upper(level) = 'VIP' THEN 0 ELSE 1 END, last_follow_time DESC, id ASC"
    ));
    query.addBindValue(salesId);
    if (query.exec()) {
        while (query.next()) {
            customers.push_back(customerFromQuery(query));
        }
    }
    return customers;
}

std::vector<Customer> SQLiteCustomerRepo::getCustomersByDepartment(const QString& department)
{
    std::vector<Customer> customers;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT DISTINCT c.* FROM customer c "
        "LEFT JOIN user u ON c.owner_id = u.id "
        "WHERE u.department = ? AND c.owner_id IS NOT NULL AND c.owner_id != '' "
        "ORDER BY CASE WHEN upper(c.level) = 'VIP' THEN 0 ELSE 1 END, c.last_follow_time DESC, c.id ASC"
    ));
    query.addBindValue(department);
    if (query.exec()) {
        while (query.next()) {
            customers.push_back(customerFromQuery(query));
        }
    }
    return customers;
}

std::vector<Customer> SQLiteCustomerRepo::searchCustomers(const QString& keyword, const User& currentUser)
{
    std::vector<Customer> customers;
    QSqlQuery query(m_db);
    QString sql = QStringLiteral("SELECT DISTINCT c.* FROM customer c ");

    if (currentUser.getRole() == UserRole::Sales) {
        sql += QStringLiteral("WHERE (c.owner_id = ? OR c.owner_id = '' OR c.owner_id IS NULL) AND ");
    } else if (currentUser.getRole() == UserRole::Manager) {
        sql += QStringLiteral("LEFT JOIN user u ON c.owner_id = u.id ");
        sql += QStringLiteral("WHERE (c.owner_id = '' OR c.owner_id IS NULL OR u.department = ?) AND ");
    } else {
        sql += QStringLiteral("WHERE ");
    }

    sql += QStringLiteral(
        "(c.name LIKE ? OR c.phone LIKE ? OR c.id LIKE ?) "
        "ORDER BY CASE WHEN upper(c.level) = 'VIP' THEN 0 ELSE 1 END, c.last_follow_time DESC, c.id ASC"
    );
    query.prepare(sql);

    if (currentUser.getRole() == UserRole::Sales) {
        query.addBindValue(currentUser.getUserId());
    } else if (currentUser.getRole() == UserRole::Manager) {
        query.addBindValue(currentUser.getDepartment());
    }

    const QString likeKeyword = QStringLiteral("%") + keyword + QStringLiteral("%");
    query.addBindValue(likeKeyword);
    query.addBindValue(likeKeyword);
    query.addBindValue(likeKeyword);

    if (query.exec()) {
        while (query.next()) {
            customers.push_back(customerFromQuery(query));
        }
    }
    return customers;
}

bool SQLiteCustomerRepo::saveCustomer(const Customer& customer)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO customer (id, name, phone, level, last_follow_time, owner_id) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ));
    query.addBindValue(customer.getId());
    query.addBindValue(customer.getName());
    query.addBindValue(customer.getPhone());
    query.addBindValue(customer.getLevel().compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0
                           ? QStringLiteral("VIP")
                           : QStringLiteral("\u666e\u901a"));
    query.addBindValue(customer.getLastFollowTime().toMSecsSinceEpoch());
    query.addBindValue(customer.getOwnerId());
    return query.exec();
}

bool SQLiteCustomerRepo::deleteCustomer(const QString& customerId)
{
    DatabaseManager& databaseManager = DatabaseManager::instance();
    if (!databaseManager.beginTransaction()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM follow_record WHERE customer_id = ?"));
    query.addBindValue(customerId);
    if (!query.exec()) {
        databaseManager.rollbackTransaction();
        return false;
    }

    query.prepare(QStringLiteral("DELETE FROM customer WHERE id = ?"));
    query.addBindValue(customerId);
    if (!query.exec()) {
        databaseManager.rollbackTransaction();
        return false;
    }

    return databaseManager.commitTransaction();
}

std::vector<FollowRecord> SQLiteCustomerRepo::getFollowRecords(const QString& customerId)
{
    std::vector<FollowRecord> records;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT * FROM follow_record WHERE customer_id = ? ORDER BY date DESC"));
    query.addBindValue(customerId);
    if (query.exec()) {
        while (query.next()) {
            records.push_back(followRecordFromQuery(query));
        }
    }
    return records;
}

bool SQLiteCustomerRepo::insertFollowRecord(const FollowRecord& record)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO follow_record (id, customer_id, operator_id, operator_name, content, date) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    ));
    query.addBindValue(record.getRecordId());
    query.addBindValue(record.getCustomerId());
    query.addBindValue(record.getOperatorId());
    query.addBindValue(record.getOperatorName());
    query.addBindValue(record.getContent());
    query.addBindValue(record.getDate().toMSecsSinceEpoch());
    return query.exec();
}

bool SQLiteCustomerRepo::assignSalesToDepartment(const QString& salesId, const QString& targetDepartment)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE user SET department = ? WHERE id = ?"));
    query.addBindValue(targetDepartment);
    query.addBindValue(salesId);
    return query.exec();
}

bool SQLiteCustomerRepo::transferCustomers(const QString& fromSalesId, const QString& toSalesId)
{
    DatabaseManager& databaseManager = DatabaseManager::instance();
    if (!databaseManager.beginTransaction()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE customer SET owner_id = ? WHERE owner_id = ?"));
    query.addBindValue(toSalesId);
    query.addBindValue(fromSalesId);
    if (!query.exec()) {
        databaseManager.rollbackTransaction();
        return false;
    }

    return databaseManager.commitTransaction();
}

std::vector<Customer> SQLiteCustomerRepo::getHighSeasCustomers()
{
    std::vector<Customer> customers;
    QSqlQuery query(QStringLiteral(
        "SELECT * FROM customer WHERE owner_id = '' OR owner_id IS NULL "
        "ORDER BY CASE WHEN upper(level) = 'VIP' THEN 0 ELSE 1 END, last_follow_time DESC, id ASC"
    ), m_db);
    while (query.next()) {
        customers.push_back(customerFromQuery(query));
    }
    return customers;
}

bool SQLiteCustomerRepo::claimCustomer(const QString& customerId, const QString& salesId)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE customer SET owner_id = ? WHERE id = ? AND (owner_id = '' OR owner_id IS NULL)"));
    query.addBindValue(salesId);
    query.addBindValue(customerId);
    return query.exec();
}

bool SQLiteCustomerRepo::releaseCustomerToHighSeas(const QString& customerId, const QString& salesId)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE customer SET owner_id = '' WHERE id = ? AND owner_id = ?"));
    query.addBindValue(customerId);
    query.addBindValue(salesId);
    return query.exec();
}
