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