#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>
#include <QDateTime>
#include <QRegularExpression>

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

#endif // CUSTOMER_H
