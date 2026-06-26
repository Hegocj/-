/**
 * @file followtimelinedialog.h
 * @brief 客户跟进时间线弹窗的类声明。
 *
 * 研究定位：本文件属于表示层中的跟进记录模块，负责定义客户沟通历史的查看与新增
 * 入口。它把离散的 FollowRecord 数据组织成用户可以阅读的时间线。
 *
 * 主要职责：声明时间线渲染和新增跟进记录方法，保存目标客户、当前用户和仓库依赖。
 * 该弹窗由客户详情页或主窗口公共工具函数打开。
 */
#ifndef FOLLOWTIMELINEDIALOG_H
#define FOLLOWTIMELINEDIALOG_H

#include <QDialog>
#include <memory>
#include "ICustomerRepository.h"

class QTextBrowser;

class FollowTimelineDialog : public QDialog
{
    Q_OBJECT

public:
    FollowTimelineDialog(const QString& customerId,
                         std::shared_ptr<ICustomerRepository> repo,
                         const User& currentUser,
                         QWidget *parent = nullptr);
    ~FollowTimelineDialog() override = default;

private slots:
    void handlePlusAppendAction(); // [+] 按钮被点击时触发

private:
    void renderTimelineAxis();     // 流式清洗并向纵向文本器刷数据

private:
    QString m_customerId;
    std::shared_ptr<ICustomerRepository> m_repo;
    User m_currentUser;
    QTextBrowser* m_axisViewer;
};

#endif // FOLLOWTIMELINEDIALOG_H
