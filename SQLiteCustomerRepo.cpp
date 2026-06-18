#include "SQLiteCustomerRepo.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QDateTime>
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
    if (m_db.isOpen()) {
        closeDatabase();
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "Database open error:" << m_db.lastError();
        return false;
    }

    if (!createTables()) {
        return false;
    }

    if (!insertInitialData()) {
        return false;
    }

    m_initialized = true;
    return true;
}

void SQLiteCustomerRepo::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool SQLiteCustomerRepo::createTables()
{
    QSqlQuery query(m_db);

    // 创建 user 表
    const QString createUserTable = R"(
        CREATE TABLE IF NOT EXISTS user (
            id TEXT PRIMARY KEY,
            username TEXT NOT NULL,
            department TEXT NOT NULL,
            role TEXT NOT NULL,
            password TEXT NOT NULL,
            is_active INTEGER NOT NULL DEFAULT 1
        )
    )";

    if (!query.exec(createUserTable)) {
        qDebug() << "Create user table error:" << query.lastError();
        return false;
    }

    // 创建 customer 表
    const QString createCustomerTable = R"(
        CREATE TABLE IF NOT EXISTS customer (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            phone TEXT NOT NULL,
            level TEXT,
            last_follow_time INTEGER,
            owner_id TEXT
        )
    )";

    if (!query.exec(createCustomerTable)) {
        qDebug() << "Create customer table error:" << query.lastError();
        return false;
    }

    // 创建 follow_record 表
    const QString createFollowRecordTable = R"(
        CREATE TABLE IF NOT EXISTS follow_record (
            id TEXT PRIMARY KEY,
            customer_id TEXT NOT NULL,
            operator_id TEXT NOT NULL,
            operator_name TEXT NOT NULL,
            content TEXT NOT NULL,
            date INTEGER NOT NULL,
            FOREIGN KEY (customer_id) REFERENCES customer(id)
        )
    )";

    if (!query.exec(createFollowRecordTable)) {
        qDebug() << "Create follow_record table error:" << query.lastError();
        return false;
    }

    return true;
}

bool SQLiteCustomerRepo::insertInitialData()
{
    QSqlQuery query(m_db);

    // 检查是否已有数据
    query.exec("SELECT COUNT(*) FROM user");
    if (query.next() && query.value(0).toInt() > 0) {
        return true; // 已有数据，不需要插入
    }

    // 插入初始用户
    const QString insertUser = R"(
        INSERT OR REPLACE INTO user (id, username, department, role, password, is_active)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    query.prepare(insertUser);
    query.addBindValue("admin");
    query.addBindValue("admin");
    query.addBindValue("总部");
    query.addBindValue("admin");
    query.addBindValue(hashPassword("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert admin user error:" << query.lastError();
        return false;
    }

    query.prepare(insertUser);
    query.addBindValue("sales01");
    query.addBindValue("sales01");
    query.addBindValue("销售部");
    query.addBindValue("sales");
    query.addBindValue(hashPassword("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert sales01 user error:" << query.lastError();
        return false;
    }

    query.prepare(insertUser);
    query.addBindValue("mgr_001");
    query.addBindValue("mgr_001");
    query.addBindValue("销售部");
    query.addBindValue("manager");
    query.addBindValue(hashPassword("123"));
    query.addBindValue(1);
    if (!query.exec()) {
        qDebug() << "Insert mgr_001 user error:" << query.lastError();
        return false;
    }

    // 插入初始客户
    const QString insertCustomer = R"(
        INSERT OR REPLACE INTO customer (id, name, phone, level, last_follow_time, owner_id)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    query.prepare(insertCustomer);
    query.addBindValue("C001");
    query.addBindValue("张三");
    query.addBindValue("13800138000");
    query.addBindValue("VIP");
    query.addBindValue(QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.addBindValue("sales01");
    if (!query.exec()) {
        qDebug() << "Insert C001 customer error:" << query.lastError();
        return false;
    }

    query.prepare(insertCustomer);
    query.addBindValue("C002");
    query.addBindValue("李四");
    query.addBindValue("13900139000");
    query.addBindValue("普通");
    query.addBindValue(QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.addBindValue("");
    if (!query.exec()) {
        qDebug() << "Insert C002 customer error:" << query.lastError();
        return false;
    }

    // 插入初始跟进记录
    const QString insertFollow = R"(
        INSERT OR REPLACE INTO follow_record (id, customer_id, operator_id, operator_name, content, date)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    query.prepare(insertFollow);
    query.addBindValue("FR001");
    query.addBindValue("C001");
    query.addBindValue("sales01");
    query.addBindValue("sales01");
    query.addBindValue("客户初次电话回访");
    query.addBindValue(QDateTime::fromString("2026-06-17", "yyyy-MM-dd").toMSecsSinceEpoch());
    if (!query.exec()) {
        qDebug() << "Insert follow record error:" << query.lastError();
        return false;
    }

    return true;
}

QString SQLiteCustomerRepo::hashPassword(const QString& password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

User SQLiteCustomerRepo::userFromQuery(const QSqlQuery& query)
{
    User user;
    user.setUserId(query.value("id").toString());
    user.setUsername(query.value("username").toString());
    user.setDepartment(query.value("department").toString());
    
    QString roleStr = query.value("role").toString();
    if (roleStr == "admin") {
        user.setRole(UserRole::Admin);
    } else if (roleStr == "manager") {
        user.setRole(UserRole::Manager);
    } else {
        user.setRole(UserRole::Sales);
    }
    
    user.setActive(query.value("is_active").toBool());
    return user;
}

Customer SQLiteCustomerRepo::customerFromQuery(const QSqlQuery& query)
{
    Customer customer;
    customer.setId(query.value("id").toString());
    customer.setName(query.value("name").toString());
    customer.setPhone(query.value("phone").toString());
    customer.setLevel(query.value("level").toString());
    customer.setLastFollowTime(QDateTime::fromMSecsSinceEpoch(query.value("last_follow_time").toLongLong()));
    customer.setOwnerId(query.value("owner_id").toString());
    return customer;
}

FollowRecord SQLiteCustomerRepo::followRecordFromQuery(const QSqlQuery& query)
{
    FollowRecord record;
    record.setRecordId(query.value("id").toString());
    record.setCustomerId(query.value("customer_id").toString());
    record.setOperatorId(query.value("operator_id").toString());
    record.setOperatorName(query.value("operator_name").toString());
    record.setContent(query.value("content").toString());
    record.setDate(QDateTime::fromMSecsSinceEpoch(query.value("date").toLongLong()));
    return record;
}

bool SQLiteCustomerRepo::checkLogin(const QString& username, const QString& password, User& outUser)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM user WHERE username = ? AND password = ? AND is_active = 1");
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));

    if (query.exec() && query.next()) {
        outUser = userFromQuery(query);
        return true;
    }
    return false;
}

std::vector<User> SQLiteCustomerRepo::getAllUsers()
{
    std::vector<User> users;
    QSqlQuery query("SELECT * FROM user", m_db);

    while (query.next()) {
        users.push_back(userFromQuery(query));
    }
    return users;
}

bool SQLiteCustomerRepo::updateUser(const User& user)
{
    QSqlQuery query(m_db);
    QString roleStr;
    switch (user.getRole()) {
        case UserRole::Admin: roleStr = "admin"; break;
        case UserRole::Manager: roleStr = "manager"; break;
        case UserRole::Sales: roleStr = "sales"; break;
    }

    query.prepare(R"(
        UPDATE user SET username = ?, department = ?, role = ?, is_active = ?
        WHERE id = ?
    )");
    query.addBindValue(user.getUsername());
    query.addBindValue(user.getDepartment());
    query.addBindValue(roleStr);
    query.addBindValue(user.isActive());
    query.addBindValue(user.getUserId());
    return query.exec();
}

std::vector<Customer> SQLiteCustomerRepo::getAllCustomers()
{
    std::vector<Customer> customers;
    QSqlQuery query("SELECT * FROM customer", m_db);

    while (query.next()) {
        customers.push_back(customerFromQuery(query));
    }
    return customers;
}

std::vector<Customer> SQLiteCustomerRepo::getCustomersBySales(const QString& salesId)
{
    std::vector<Customer> customers;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM customer WHERE owner_id = ?");
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
    query.prepare(R"(
        SELECT DISTINCT c.* FROM customer c
        LEFT JOIN user u ON c.owner_id = u.id
        WHERE u.department = ? AND c.owner_id != ''
    )");
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
    QString sql = "SELECT DISTINCT c.* FROM customer c ";
    
    if (currentUser.getRole() == UserRole::Sales) {
        sql += "WHERE (c.owner_id = ? OR c.owner_id = '') AND ";
    } else if (currentUser.getRole() == UserRole::Manager) {
        sql += "LEFT JOIN user u ON c.owner_id = u.id ";
        sql += "WHERE (c.owner_id = '' OR u.department = ?) AND ";
    } else {
        sql += "WHERE ";
    }
    
    sql += "(c.name LIKE ? OR c.phone LIKE ?)";
    query.prepare(sql);

    int paramIndex = 0;
    if (currentUser.getRole() == UserRole::Sales) {
        query.bindValue(paramIndex++, currentUser.getUserId());
    } else if (currentUser.getRole() == UserRole::Manager) {
        query.bindValue(paramIndex++, currentUser.getDepartment());
    }
    
    QString likeKeyword = "%" + keyword + "%";
    query.bindValue(paramIndex++, likeKeyword);
    query.bindValue(paramIndex++, likeKeyword);

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
    query.prepare(R"(
        INSERT OR REPLACE INTO customer (id, name, phone, level, last_follow_time, owner_id)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(customer.getId());
    query.addBindValue(customer.getName());
    query.addBindValue(customer.getPhone());
    query.addBindValue(customer.getLevel());
    query.addBindValue(customer.getLastFollowTime().toMSecsSinceEpoch());
    query.addBindValue(customer.getOwnerId());
    return query.exec();
}

bool SQLiteCustomerRepo::deleteCustomer(const QString& customerId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM customer WHERE id = ?");
    query.addBindValue(customerId);
    return query.exec();
}

std::vector<FollowRecord> SQLiteCustomerRepo::getFollowRecords(const QString& customerId)
{
    std::vector<FollowRecord> records;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM follow_record WHERE customer_id = ? ORDER BY date DESC");
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
    query.prepare(R"(
        INSERT OR REPLACE INTO follow_record (id, customer_id, operator_id, operator_name, content, date)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
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
    query.prepare("UPDATE user SET department = ? WHERE id = ?");
    query.addBindValue(targetDepartment);
    query.addBindValue(salesId);
    return query.exec();
}

bool SQLiteCustomerRepo::transferCustomers(const QString& fromSalesId, const QString& toSalesId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE customer SET owner_id = ? WHERE owner_id = ?");
    query.addBindValue(toSalesId);
    query.addBindValue(fromSalesId);
    return query.exec();
}

std::vector<Customer> SQLiteCustomerRepo::getHighSeasCustomers()
{
    std::vector<Customer> customers;
    QSqlQuery query("SELECT * FROM customer WHERE owner_id = ''", m_db);

    while (query.next()) {
        customers.push_back(customerFromQuery(query));
    }
    return customers;
}

bool SQLiteCustomerRepo::claimCustomer(const QString& customerId, const QString& salesId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE customer SET owner_id = ? WHERE id = ? AND owner_id = ''");
    query.addBindValue(salesId);
    query.addBindValue(customerId);
    return query.exec();
}

bool SQLiteCustomerRepo::releaseCustomerToHighSeas(const QString& customerId, const QString& salesId)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE customer SET owner_id = '' WHERE id = ? AND owner_id = ?");
    query.addBindValue(customerId);
    query.addBindValue(salesId);
    return query.exec();
}
