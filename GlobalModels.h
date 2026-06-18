#ifndef GLOBALMODELS_H
#define GLOBALMODELS_H

#include <QString>
#include <QDateTime>
#include <QRegularExpression>
#include <QCryptographicHash>

/**
 * @brief 用户角色枚举
 */
enum class UserRole {
    Sales,      // 销售
    Manager,    // 经理
    Admin       // 管理员
};

/**
 * @brief 员工实体类
 */
class User {
public:
    User() : m_role(UserRole::Sales), m_isActive(true) {}

    User(const QString& id, const QString& name, const QString& dept, UserRole role)
        : m_userId(id), m_username(name), m_department(dept), m_role(role), m_isActive(true) {}

    // Getter / Setter
    QString getUserId() const { return m_userId; }
    void setUserId(const QString& id) { m_userId = id; }

    QString getUsername() const { return m_username; }
    void setUsername(const QString& name) { m_username = name; }

    QString getDepartment() const { return m_department; }
    void setDepartment(const QString& dept) { m_department = dept; }

    UserRole getRole() const { return m_role; }
    void setRole(UserRole role) { m_role = role; }

    // 账户状态：离职/禁用逻辑
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

private:
    QString m_userId;
    QString m_username;
    QString m_passwordHash; // 数据库存储此字段
    QString m_department;
    UserRole m_role;
    bool m_isActive;        // 是否在职（离职自动设为 false）
};

/**
 * @brief 客户实体类
 */
class Customer {
public:
    Customer() = default;
    Customer(const QString& id, const QString& name, const QString& phone, const QString& level, const QDateTime& lastTime, const QString& ownerId)
        : m_id(id), m_name(name), m_phone(phone), m_level(level), m_lastFollowTime(lastTime), m_ownerId(ownerId) {}

    QString getId() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QString getPhone() const { return m_phone; }
    void setPhone(const QString& phone) { m_phone = phone; }

    QString getLevel() const { return m_level; }
    void setLevel(const QString& level) { m_level = level; }

    QDateTime getLastFollowTime() const { return m_lastFollowTime; }
    void setLastFollowTime(const QDateTime& time) { m_lastFollowTime = time; }

    QString getOwnerId() const { return m_ownerId; }
    void setOwnerId(const QString& ownerId) { m_ownerId = ownerId; }

    bool isValid() const {
        if (m_name.trimmed().isEmpty()) return false;
        QRegularExpression regex("^1[3-9]\\d{9}$");
        return regex.match(m_phone).hasMatch();
    }

private:
    QString m_id;
    QString m_name;
    QString m_phone;
    QString m_level;
    QDateTime m_lastFollowTime;
    QString m_ownerId;
};

/**
 * @brief 跟进记录类
 */
class FollowRecord {
public:
    FollowRecord() = default;
    FollowRecord(const QString& rId, const QString& cId, const QString& opId, const QString& opName, const QString& ctx, const QDateTime& t)
        : m_recordId(rId), m_customerId(cId), m_operatorId(opId), m_operatorName(opName), m_content(ctx), m_date(t) {}

    QString getRecordId() const { return m_recordId; }
    void setRecordId(const QString& id) { m_recordId = id; }

    QString getCustomerId() const { return m_customerId; }
    void setCustomerId(const QString& id) { m_customerId = id; }

    QString getOperatorId() const { return m_operatorId; }
    void setOperatorId(const QString& id) { m_operatorId = id; }

    QString getOperatorName() const { return m_operatorName; }
    void setOperatorName(const QString& name) { m_operatorName = name; }

    QString getContent() const { return m_content; }
    void setContent(const QString& ctx) { m_content = ctx; }

    QDateTime getDate() const { return m_date; }
    void setDate(const QDateTime& t) { m_date = t; }

private:
    QString m_recordId;
    QString m_customerId;
    QString m_operatorId;
    QString m_operatorName;
    QString m_content;
    QDateTime m_date;
};

#endif // GLOBALMODELS_H