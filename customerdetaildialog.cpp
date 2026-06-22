#include "customerdetaildialog.h"
#include "followtimelinedialog.h"

#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CustomerDetailDialog::CustomerDetailDialog(const QString& customerId,
                                           std::shared_ptr<ICustomerRepository> repo,
                                           const User& currentUser,
                                           QWidget *parent)
    : QDialog(parent)
    , m_customerId(customerId)
    , m_repo(repo)
    , m_currentUser(currentUser)
{
    setWindowTitle(QStringLiteral("\u5ba2\u6237\u8be6\u60c5"));
    resize(420, 340);

    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(this);
    m_idEdit->setReadOnly(true);
    m_nameEdit = new QLineEdit(this);
    m_phoneEdit = new QLineEdit(this);
    m_levelCombo = new QComboBox(this);
    m_levelCombo->addItem(QStringLiteral("\u666e\u901a"));
    m_levelCombo->addItem(QStringLiteral("VIP"));
    m_ownerCombo = new QComboBox(this);

    formLayout->addRow(QStringLiteral("\u5ba2\u6237ID:"), m_idEdit);
    formLayout->addRow(QStringLiteral("\u5ba2\u6237\u59d3\u540d:"), m_nameEdit);
    formLayout->addRow(QStringLiteral("\u8054\u7cfb\u7535\u8bdd:"), m_phoneEdit);
    formLayout->addRow(QStringLiteral("\u5ba2\u6237\u7b49\u7ea7:"), m_levelCombo);
    formLayout->addRow(QStringLiteral("\u8d1f\u8d23\u9500\u552e:"), m_ownerCombo);
    mainLayout->addLayout(formLayout);

    m_followBtn = new QPushButton(QStringLiteral("\u67e5\u770b\u548c\u6dfb\u52a0\u8ddf\u8fdb\u8bb0\u5f55"), this);
    mainLayout->addWidget(m_followBtn);

    auto* featureLayout = new QHBoxLayout();
    m_claimBtn = new QPushButton(QStringLiteral("\u8ba4\u9886\u5ba2\u6237"), this);
    m_evictBtn = new QPushButton(QStringLiteral("\u91ca\u653e\u5230\u516c\u6d77\u6c60"), this);
    m_deleteBtn = new QPushButton(QStringLiteral("\u5220\u9664\u5ba2\u6237"), this);
    m_deleteBtn->setStyleSheet(QStringLiteral("QPushButton { background-color: #d9534f; color: white; font-weight: bold; padding: 6px; }"));
    m_claimBtn->hide();
    m_evictBtn->hide();
    if (m_currentUser.getRole() != UserRole::Admin) {
        m_deleteBtn->hide();
    }
    featureLayout->addWidget(m_claimBtn);
    featureLayout->addWidget(m_evictBtn);
    featureLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(featureLayout);

    auto* bottomLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("\u4fdd\u5b58"), this);
    m_cancelBtn = new QPushButton(QStringLiteral("\u53d6\u6d88"), this);
    bottomLayout->addWidget(m_saveBtn);
    bottomLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(bottomLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_followBtn, &QPushButton::clicked, this, &CustomerDetailDialog::openFollowTimeline);
    connect(m_saveBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleSaveOrCommit);
    connect(m_claimBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleClaimAction);
    connect(m_evictBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleEvictAction);
    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        const auto button = QMessageBox::critical(
            this,
            QStringLiteral("\u8b66\u544a"),
            QStringLiteral("\u786e\u5b9a\u8981\u5220\u9664\u8be5\u5ba2\u6237\u5417\uff1f"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (button == QMessageBox::Yes) {
            m_repo->deleteCustomer(m_customerId);
            QMessageBox::information(this,
                                     QStringLiteral("\u6210\u529f"),
                                     QStringLiteral("\u5ba2\u6237\u6570\u636e\u5df2\u5220\u9664\u3002"));
            accept();
        }
    });

    loadCustomerAndSetupPrivilege();
}

void CustomerDetailDialog::loadCustomerAndSetupPrivilege()
{
    for (const auto& customer : m_repo->getAllCustomers()) {
        if (customer.getId() == m_customerId) {
            m_customer = customer;
            break;
        }
    }

    m_idEdit->setText(m_customer.getId());
    m_nameEdit->setText(m_customer.getName());
    m_phoneEdit->setText(m_customer.getPhone());
    const int levelIndex = m_levelCombo->findText(m_customer.getLevel(), Qt::MatchFixedString);
    m_levelCombo->setCurrentIndex(levelIndex >= 0 ? levelIndex : 0);

    const QString ownerId = m_customer.getOwnerId();

    if (m_currentUser.getRole() == UserRole::Sales) {
        m_ownerCombo->addItem(ownerId.isEmpty() ? QStringLiteral("\u516c\u6d77\u6c60") : ownerId, ownerId);
        m_ownerCombo->setEnabled(false);

        if (ownerId.isEmpty()) {
            m_nameEdit->setReadOnly(true);
            m_phoneEdit->setReadOnly(true);
            m_levelCombo->setEnabled(false);
            m_followBtn->setEnabled(false);
            m_saveBtn->hide();
            m_claimBtn->show();
        } else if (ownerId == m_currentUser.getUserId()) {
            m_nameEdit->setReadOnly(true);
            m_levelCombo->setEnabled(false);
            m_phoneEdit->setReadOnly(false);
            m_followBtn->setEnabled(true);
            m_saveBtn->show();
        } else {
            m_nameEdit->setReadOnly(true);
            m_phoneEdit->setReadOnly(true);
            m_levelCombo->setEnabled(false);
            m_followBtn->setEnabled(false);
            m_saveBtn->hide();
        }
        return;
    }

    m_nameEdit->setReadOnly(false);
    m_phoneEdit->setReadOnly(false);
    m_levelCombo->setEnabled(true);
    m_followBtn->setEnabled(true);
    m_saveBtn->show();
    populateSalesCombo();
    m_ownerCombo->setEnabled(true);

    if (!ownerId.isEmpty()) {
        m_evictBtn->show();
    }
}

void CustomerDetailDialog::populateSalesCombo()
{
    m_ownerCombo->clear();
    m_ownerCombo->addItem(QStringLiteral("\u516c\u6d77\u6c60"), QString());

    int targetIndex = 0;
    int currentIndex = 1;
    for (const auto& user : m_repo->getAllUsers()) {
        const bool visible = (m_currentUser.getRole() == UserRole::Admin) ||
                             (user.getDepartment() == m_currentUser.getDepartment());
        if (visible && user.getRole() == UserRole::Sales && user.isActive()) {
            m_ownerCombo->addItem(QStringLiteral("%1 (ID: %2)").arg(user.getUsername(), user.getUserId()), user.getUserId());
            if (user.getUserId() == m_customer.getOwnerId()) {
                targetIndex = currentIndex;
            }
            ++currentIndex;
        }
    }
    m_ownerCombo->setCurrentIndex(targetIndex);
}

void CustomerDetailDialog::handleSaveOrCommit()
{
    m_customer.setName(m_nameEdit->text().trimmed());
    m_customer.setPhone(m_phoneEdit->text().trimmed());
    m_customer.setLevel(m_levelCombo->currentText());

    if (m_currentUser.getRole() == UserRole::Manager || m_currentUser.getRole() == UserRole::Admin) {
        m_customer.setOwnerId(m_ownerCombo->currentData().toString());
    }

    if (m_repo->saveCustomer(m_customer)) {
        QMessageBox::information(this,
                                 QStringLiteral("\u63d0\u793a"),
                                 QStringLiteral("\u5ba2\u6237\u4fe1\u606f\u4fdd\u5b58\u6210\u529f\u3002"));
        accept();
    } else {
        QMessageBox::critical(this,
                              QStringLiteral("\u9519\u8bef"),
                              QStringLiteral("\u4fdd\u5b58\u5931\u8d25\u3002"));
    }
}

void CustomerDetailDialog::handleClaimAction()
{
    if (m_repo->claimCustomer(m_customer.getId(), m_currentUser.getUserId())) {
        QMessageBox::information(this,
                                 QStringLiteral("\u6210\u529f"),
                                 QStringLiteral("\u5ba2\u6237\u8ba4\u9886\u6210\u529f\u3002"));
        accept();
    } else {
        QMessageBox::warning(this,
                             QStringLiteral("\u5931\u8d25"),
                             QStringLiteral("\u8ba4\u9886\u5931\u8d25\uff0c\u53ef\u80fd\u5ba2\u6237\u5df2\u88ab\u5176\u4ed6\u4eba\u8ba4\u9886\u3002"));
        reject();
    }
}

void CustomerDetailDialog::handleEvictAction()
{
    const auto reply = QMessageBox::question(
        this,
        QStringLiteral("\u786e\u8ba4\u91ca\u653e"),
        QStringLiteral("\u786e\u5b9a\u8981\u5c06\u8be5\u5ba2\u6237\u91ca\u653e\u5230\u516c\u6d77\u6c60\u5417\uff1f"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_customer.setOwnerId(QString());
        if (m_repo->saveCustomer(m_customer)) {
            QMessageBox::information(this,
                                     QStringLiteral("\u6210\u529f"),
                                     QStringLiteral("\u5ba2\u6237\u5df2\u91ca\u653e\u5230\u516c\u6d77\u6c60\u3002"));
            accept();
        }
    }
}

void CustomerDetailDialog::openFollowTimeline()
{
    FollowTimelineDialog timelineDlg(m_customerId, m_repo, m_currentUser, this);
    timelineDlg.exec();
}
