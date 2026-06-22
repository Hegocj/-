#ifndef USER_H
#define USER_H

#include <QString>

enum class UserRole {
    Sales,
    Manager,
    Admin
};

class User {
public:
    User() : m_role(UserRole::Sales), m_isActive(true) {}

    User(const QString& id, const QString& name, const QString& dept, UserRole role)
        : m_userId(id)
        , m_username(name)
        , m_department(dept)
        , m_role(role)
        , m_isActive(true)
    {
    }

    QString getUserId() const { return m_userId; }
    void setUserId(const QString& id) { m_userId = id; }

    QString getUsername() const { return m_username; }
    void setUsername(const QString& name) { m_username = name; }

    QString getPassword() const { return m_password; }
    void setPassword(const QString& password) { m_password = password; }

    QString getDepartment() const { return m_department; }
    void setDepartment(const QString& dept) { m_department = dept; }

    UserRole getRole() const { return m_role; }
    void setRole(UserRole role) { m_role = role; }

    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

private:
    QString m_userId;
    QString m_username;
    QString m_password;
    QString m_department;
    UserRole m_role;
    bool m_isActive;
};

#endif
