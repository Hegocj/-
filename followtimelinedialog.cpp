#include "FollowTimelineDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QInputDialog>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>

FollowTimelineDialog::FollowTimelineDialog(const QString& customerId,
                                           std::shared_ptr<ICustomerRepository> repo,
                                           const User& currentUser,
                                           QWidget *parent)
    : QDialog(parent),
    m_customerId(customerId),
    m_repo(repo),
    m_currentUser(currentUser)
{
    setWindowTitle("Follow-up Activity Timeline Engine");
    resize(460, 480);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 顶部水平底座：标题和纯文本 [+] 号添加按钮并排
    QHBoxLayout* topHeaderLayout = new QHBoxLayout();
    topHeaderLayout->addWidget(new QLabel("Chronological Tracking Axis:", this));

    QPushButton* plusButton = new QPushButton("[+] Add Follow Log", this);
    plusButton->setFixedWidth(130);
    topHeaderLayout->addWidget(plusButton);
    mainLayout->addLayout(topHeaderLayout);

    // 纵向时间轴文本滚屏视窗
    m_axisViewer = new QTextBrowser(this);
    m_axisViewer->setReadOnly(true);
    mainLayout->addWidget(m_axisViewer);

    // 挂载 [+] 号点击业务
    connect(plusButton, &QPushButton::clicked, this, &FollowTimelineDialog::handlePlusAppendAction);

    // 刷新时间轴
    renderTimelineAxis();
}

void FollowTimelineDialog::renderTimelineAxis()
{
    m_axisViewer->clear();
    std::vector<FollowRecord> records = m_repo->getFollowRecords(m_customerId);

    if (records.empty()) {
        m_axisViewer->append(" No trackable footprints logged yet.");
        return;
    }

    for (const auto& log : records) {
        QString stamp = log.getDate().toString("yyyy-MM-dd HH:mm:ss");
        m_axisViewer->append(QString("Time Node: %1").arg(stamp));
        m_axisViewer->append(QString("Operator: %1 [ID: %2]").arg(log.getOperatorName(), log.getOperatorId()));
        m_axisViewer->append(QString("Execution Log: %1").arg(log.getContent()));
        m_axisViewer->append("==================================================");
    }
}

void FollowTimelineDialog::handlePlusAppendAction()
{
    bool ok;
    // 呼叫轻量级输入框
    QString newLogText = QInputDialog::getMultiLineText(this, "Append Tracking Record",
                                                        "Type new communication details:", "", &ok);
    if (ok && !newLogText.trimmed().isEmpty()) {
        QString recordId = "REC_AUTO_" + QString::number(QDateTime::currentMSecsSinceEpoch());

        FollowRecord freshLog(
            recordId,
            m_customerId,
            m_currentUser.getUserId(),
            m_currentUser.getUsername(),
            newLogText.trimmed(),
            QDateTime::currentDateTime()
            );

        if (m_repo->insertFollowRecord(freshLog)) {
            renderTimelineAxis(); // 原地重绘刷屏
        } else {
            QMessageBox::warning(this, "Abort", "Persistence layer validation failed.");
        }
    }
}