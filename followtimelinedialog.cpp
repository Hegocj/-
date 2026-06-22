#include "FollowTimelineDialog.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

FollowTimelineDialog::FollowTimelineDialog(const QString& customerId,
                                           std::shared_ptr<ICustomerRepository> repo,
                                           const User& currentUser,
                                           QWidget *parent)
    : QDialog(parent)
    , m_customerId(customerId)
    , m_repo(repo)
    , m_currentUser(currentUser)
{
    setWindowTitle(QStringLiteral("\u5ba2\u6237\u8ddf\u8fdb\u65f6\u95f4\u7ebf"));
    resize(460, 480);

    auto* mainLayout = new QVBoxLayout(this);
    auto* topHeaderLayout = new QHBoxLayout();
    topHeaderLayout->addWidget(new QLabel(QStringLiteral("\u65f6\u95f4\u7ebf\u8ddf\u8e2a\u8bb0\u5f55"), this));

    auto* plusButton = new QPushButton(QStringLiteral("[+] \u6dfb\u52a0\u8ddf\u8fdb\u8bb0\u5f55"), this);
    plusButton->setFixedWidth(150);
    topHeaderLayout->addWidget(plusButton);
    mainLayout->addLayout(topHeaderLayout);

    m_axisViewer = new QTextBrowser(this);
    m_axisViewer->setReadOnly(true);
    mainLayout->addWidget(m_axisViewer);

    connect(plusButton, &QPushButton::clicked, this, &FollowTimelineDialog::handlePlusAppendAction);
    renderTimelineAxis();
}

void FollowTimelineDialog::renderTimelineAxis()
{
    m_axisViewer->clear();
    const std::vector<FollowRecord> records = m_repo->getFollowRecords(m_customerId);

    if (records.empty()) {
        m_axisViewer->append(QStringLiteral("\u6682\u65e0\u8ddf\u8fdb\u8bb0\u5f55\u3002"));
        return;
    }

    for (const auto& log : records) {
        const QString stamp = log.getDate().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        m_axisViewer->append(QStringLiteral("\u65f6\u95f4\u8282\u70b9: %1").arg(stamp));
        m_axisViewer->append(QStringLiteral("\u64cd\u4f5c\u4eba\u5458: %1 [ID: %2]").arg(log.getOperatorName(), log.getOperatorId()));
        m_axisViewer->append(QStringLiteral("\u8ddf\u8fdb\u5185\u5bb9: %1").arg(log.getContent()));
        m_axisViewer->append(QStringLiteral("=================================================="));
    }
}

void FollowTimelineDialog::handlePlusAppendAction()
{
    bool ok = false;
    const QString newLogText = QInputDialog::getMultiLineText(
        this,
        QStringLiteral("\u6dfb\u52a0\u8ddf\u8fdb\u8bb0\u5f55"),
        QStringLiteral("\u8bf7\u8f93\u5165\u65b0\u7684\u6c9f\u901a\u5185\u5bb9"),
        QString(),
        &ok
    );

    if (!ok || newLogText.trimmed().isEmpty()) {
        return;
    }

    FollowRecord freshLog(
        QStringLiteral("REC_AUTO_%1").arg(QDateTime::currentMSecsSinceEpoch()),
        m_customerId,
        m_currentUser.getUserId(),
        m_currentUser.getUsername(),
        newLogText.trimmed(),
        QDateTime::currentDateTime()
    );

    if (m_repo->insertFollowRecord(freshLog)) {
        renderTimelineAxis();
    } else {
        QMessageBox::warning(this,
                             QStringLiteral("\u5931\u8d25"),
                             QStringLiteral("\u4fdd\u5b58\u8bb0\u5f55\u5931\u8d25\u3002"));
    }
}
