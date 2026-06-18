#ifndef FOLLOWRECORD_H
#define FOLLOWRECORD_H

#include <QString>
#include <QDateTime>

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

#endif // FOLLOWRECORD_H
