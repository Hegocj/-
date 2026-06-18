#ifndef USER_H
#define USER_H

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

#endif
