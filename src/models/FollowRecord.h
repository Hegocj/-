/**
 * @file FollowRecord.h
 * @brief 客户跟进记录实体模型定义。
 *
 * 研究定位：本文件属于模型层，用于描述客户在销售过程中的历史沟通、维护和操作
 * 记录。FollowRecord 是客户时间线展示和新增跟进内容的基础数据结构。
 *
 * 主要职责：保存记录编号、客户编号、操作人编号、操作人姓名、跟进内容和记录时间，
 * 供仓库层持久化，也供 FollowTimelineDialog 按时间顺序展示。
 */
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
