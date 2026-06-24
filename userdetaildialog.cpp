#include "userdetaildialog.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>

#include <utility>

UserDetailDialog::UserDetailDialog(const QString& targetUserId,
                                   std::shared_ptr<ICustomerRepository> repo,
                                   const User& currentUser,
                                   QWidget* parent)
    : QDialog(parent)
    , m_targetUserId(targetUserId)
    , m_repo(std::move(repo))
    , m_currentUser(currentUser)
{
    setupUi();
    loadUserData();
}

void UserDetailDialog::setupUi()
{
    setWindowTitle(QStringLiteral("\u5458\u5de5\u6863\u6848\u4e0e\u8d26\u53f7\u7ef4\u62a4"));
    resize(460, 360);

    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(this);
    m_nameEdit = new QLineEdit(this);
    m_deptEdit = new QLineEdit(this);
    m_roleCombo = new QComboBox(this);
    m_statusEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);

    m_idEdit->setReadOnly(true);
    m_roleCombo->addItem(QStringLiteral("\u9500\u552e"), static_cast<int>(UserRole::Sales));
    m_roleCombo->addItem(QStringLiteral("\u7ecf\u7406"), static_cast<int>(UserRole::Manager));
    m_roleCombo->addItem(QStringLiteral("\u7ba1\u7406\u5458"), static_cast<int>(UserRole::Admin));
    m_statusEdit->setReadOnly(true);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QStringLiteral("\u7559\u7a7a\u8868\u793a\u4e0d\u4fee\u6539\u5bc6\u7801"));

    formLayout->addRow(QStringLiteral("\u5458\u5de5\u5de5\u53f7:"), m_idEdit);
    formLayout->addRow(QStringLiteral("\u767b\u5f55\u7528\u6237\u540d:"), m_nameEdit);
    formLayout->addRow(QStringLiteral("\u6240\u5c5e\u90e8\u95e8:"), m_deptEdit);
    formLayout->addRow(QStringLiteral("\u7cfb\u7edf\u89d2\u8272:"), m_roleCombo);
    formLayout->addRow(QStringLiteral("\u8d26\u53f7\u72b6\u6001:"), m_statusEdit);
    formLayout->addRow(QStringLiteral("\u65b0\u5bc6\u7801:"), m_passwordEdit);
    mainLayout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("\u4fdd\u5b58\u4fee\u6539"), this);
    m_transferBtn = new QPushButton(QStringLiteral("\u529e\u7406\u79bb\u804c\u5e76\u8f6c\u79fb\u5ba2\u6237"), this);
    m_deleteBtn = new QPushButton(QStringLiteral("\u5f7b\u5e95\u5220\u9664\u5458\u5de5\u6863\u6848"), this);
    m_closeBtn = new QPushButton(QStringLiteral("\u5173\u95ed"), this);

    m_transferBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; font-weight: bold; padding: 6px; }");
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; font-weight: bold; padding: 6px; }");

    if (m_currentUser.getRole() == UserRole::Sales) {
        m_saveBtn->setVisible(false);
        m_transferBtn->setVisible(false);
    }
    if (m_currentUser.getRole() != UserRole::Admin) {
        m_deleteBtn->setVisible(false);
    }

    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_transferBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &UserDetailDialog::onSaveButtonClicked);
    connect(m_transferBtn, &QPushButton::clicked, this, &UserDetailDialog::onTransferButtonClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &UserDetailDialog::onDeleteButtonClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void UserDetailDialog::loadUserData()
{
    bool found = false;
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getUserId() == m_targetUserId) {
            m_targetUser = user;
            found = true;
            break;
        }
    }

    if (!found) {
        m_idEdit->setText(m_targetUserId);
        m_nameEdit->setText(QStringLiteral("\u672a\u77e5\u5458\u5de5"));
        m_statusEdit->setText(QStringLiteral("\u672a\u627e\u5230\u8be5\u5458\u5de5\u6863\u6848"));
        m_saveBtn->setEnabled(false);
        m_transferBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
        return;
    }

    m_idEdit->setText(m_targetUser.getUserId());
    m_nameEdit->setText(m_targetUser.getUsername());
    m_deptEdit->setText(m_targetUser.getDepartment());

    const int roleValue = static_cast<int>(m_targetUser.getRole());
    const int roleIndex = m_roleCombo->findData(roleValue);
    if (roleIndex >= 0) {
        m_roleCombo->setCurrentIndex(roleIndex);
    }
    m_statusEdit->setText(m_targetUser.isActive()
                              ? QStringLiteral("\u5728\u804c")
                              : QStringLiteral("\u79bb\u804c"));

    const bool canEdit = m_currentUser.getRole() == UserRole::Admin ||
                         (m_currentUser.getRole() == UserRole::Manager && m_targetUser.getRole() == UserRole::Sales);
    m_nameEdit->setReadOnly(!canEdit);
    m_deptEdit->setReadOnly(!canEdit);
    m_passwordEdit->setReadOnly(!canEdit);
    m_roleCombo->setEnabled(m_currentUser.getRole() == UserRole::Admin);
    m_saveBtn->setEnabled(canEdit);
    m_transferBtn->setEnabled(m_targetUser.isActive() &&
                              m_targetUser.getRole() == UserRole::Sales &&
                              m_currentUser.getRole() != UserRole::Sales);
}

void UserDetailDialog::onSaveButtonClicked()
{
    if (m_nameEdit->text().trimmed().isEmpty() || m_deptEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u4fdd\u5b58\u5931\u8d25"),
                             QStringLiteral("\u7528\u6237\u540d\u548c\u90e8\u95e8\u4e0d\u80fd\u4e3a\u7a7a\u3002"));
        return;
    }

    User updated = m_targetUser;
    updated.setUsername(m_nameEdit->text().trimmed());
    updated.setDepartment(m_deptEdit->text().trimmed());
    updated.setRole(static_cast<UserRole>(m_roleCombo->currentData().toInt()));
    updated.setActive(m_targetUser.isActive());

    if (!m_repo->saveUser(updated, m_passwordEdit->text())) {
        QMessageBox::critical(this,
                              QStringLiteral("\u4fdd\u5b58\u5931\u8d25"),
                              QStringLiteral("\u5458\u5de5\u4fe1\u606f\u4fdd\u5b58\u5931\u8d25\u3002"));
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u4fdd\u5b58\u6210\u529f"),
                             QStringLiteral("\u5458\u5de5\u4fe1\u606f\u5df2\u66f4\u65b0\u3002"));
    accept();
}

void UserDetailDialog::onTransferButtonClicked()
{
    if (m_currentUser.getRole() == UserRole::Sales) {
        QMessageBox::critical(this,
                              QStringLiteral("\u6743\u9650\u62e6\u622a"),
                              QStringLiteral("\u9500\u552e\u4eba\u5458\u65e0\u6743\u529e\u7406\u5458\u5de5\u79bb\u804c\u3002"));
        return;
    }

    if (m_targetUser.getRole() != UserRole::Sales) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u6cd5\u529e\u7406"),
                             QStringLiteral("\u5f53\u524d\u4ec5\u652f\u6301\u5bf9\u9500\u552e\u4eba\u5458\u529e\u7406\u5ba2\u6237\u8d44\u4ea7\u4ea4\u63a5\u3002"));
        return;
    }

    QStringList receiverItems;
    QStringList receiverIds;
    receiverItems << QStringLiteral("\u8f6c\u5165\u516c\u6d77\u6c60");
    receiverIds << QString();

    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Sales &&
            user.isActive() &&
            user.getUserId() != m_targetUserId &&
            user.getDepartment() == m_targetUser.getDepartment()) {
            receiverItems << QStringLiteral("%1\uff08%2\uff09").arg(user.getUsername(), user.getUserId());
            receiverIds << user.getUserId();
        }
    }

    bool ok = false;
    const QString receiverText = QInputDialog::getItem(
        this,
        QStringLiteral("\u9009\u62e9\u5ba2\u6237\u63a5\u6536\u4eba"),
        QStringLiteral("\u8bf7\u9009\u62e9\u8be5\u5458\u5de5\u540d\u4e0b\u5ba2\u6237\u7684\u63a5\u6536\u65b9\u5f0f\uff1a"),
        receiverItems,
        0,
        false,
        &ok
    );
    if (!ok) {
        return;
    }

    const int receiverIndex = receiverItems.indexOf(receiverText);
    const QString receiverId = receiverIds.value(receiverIndex);
    const QString confirmText = receiverId.isEmpty()
        ? QStringLiteral("\u786e\u5b9a\u5c06\u5458\u5de5 [%1] \u6807\u8bb0\u4e3a\u79bb\u804c\uff0c\u5e76\u628a\u5176\u540d\u4e0b\u5ba2\u6237\u8f6c\u5165\u516c\u6d77\u6c60\u5417\uff1f").arg(m_targetUser.getUsername())
        : QStringLiteral("\u786e\u5b9a\u5c06\u5458\u5de5 [%1] \u6807\u8bb0\u4e3a\u79bb\u804c\uff0c\u5e76\u628a\u5176\u540d\u4e0b\u5ba2\u6237\u8f6c\u4ea4\u7ed9 [%2] \u5417\uff1f").arg(m_targetUser.getUsername(), receiverText);

    if (QMessageBox::warning(this,
                             QStringLiteral("\u5458\u5de5\u79bb\u804c\u4ea4\u63a5\u786e\u8ba4"),
                             confirmText,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    bool transferOk = true;
    if (receiverId.isEmpty()) {
        for (const auto& customer : m_repo->getCustomersBySales(m_targetUserId)) {
            if (!m_repo->releaseCustomerToHighSeas(customer.getId(), m_targetUserId)) {
                Customer updated = customer;
                updated.setOwnerId(QString());
                transferOk = m_repo->saveCustomer(updated) && transferOk;
            }
        }
    } else {
        transferOk = m_repo->transferCustomers(m_targetUserId, receiverId);
    }

    User updatedUser = m_targetUser;
    updatedUser.setActive(false);
    const bool updateUserOk = m_repo->updateUser(updatedUser);

    if (!transferOk || !updateUserOk) {
        QMessageBox::critical(this,
                              QStringLiteral("\u529e\u7406\u5931\u8d25"),
                              QStringLiteral("\u79bb\u804c\u72b6\u6001\u6216\u5ba2\u6237\u8d44\u4ea7\u4ea4\u63a5\u4fdd\u5b58\u5931\u8d25\u3002"));
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u529e\u7406\u5b8c\u6210"),
                             QStringLiteral("\u5458\u5de5 [%1] \u5df2\u529e\u7406\u79bb\u804c\uff0c\u5ba2\u6237\u8d44\u4ea7\u5df2\u5b8c\u6210\u4ea4\u63a5\u3002").arg(m_targetUser.getUsername()));
    accept();
}

void UserDetailDialog::onDeleteButtonClicked()
{
    if (m_currentUser.getRole() != UserRole::Admin) {
        QMessageBox::warning(this,
                             QStringLiteral("\u6743\u9650\u62e6\u622a"),
                             QStringLiteral("\u53ea\u6709\u7ba1\u7406\u5458\u624d\u80fd\u5220\u9664\u5458\u5de5\u6863\u6848\u3002"));
        return;
    }

    if (m_targetUser.getUserId() == m_currentUser.getUserId()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u6cd5\u5220\u9664"),
                             QStringLiteral("\u4e0d\u80fd\u5220\u9664\u81ea\u5df1\u7684\u8d26\u53f7\u3002"));
        return;
    }

    const auto confirmText = QStringLiteral(
        "\u786e\u5b9a\u8981\u5f7b\u5e95\u5220\u9664\u5458\u5de5 [%1] \u7684\u6863\u6848\u5417\uff1f\n\n"
        "\u6ce8\u610f\uff1a\u8be5\u5458\u5de5\u540d\u4e0b\u7684\u6240\u6709\u5ba2\u6237\u5c06\u81ea\u52a8\u8f6c\u5165\u516c\u6d77\u6c60\uff0c\u8be5\u64cd\u4f5c\u4e0d\u53ef\u6062\u590d\uff01"
    ).arg(m_targetUser.getUsername());

    if (QMessageBox::warning(this,
                             QStringLiteral("\u786e\u8ba4\u5220\u9664"),
                             confirmText,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (!m_repo->deleteUser(m_targetUser.getUserId())) {
        QMessageBox::critical(this,
                             QStringLiteral("\u5220\u9664\u5931\u8d25"),
                             QStringLiteral("\u5458\u5de5\u6863\u6848\u5220\u9664\u5931\u8d25\u3002"));
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u5220\u9664\u6210\u529f"),
                             QStringLiteral("\u5458\u5de5 [%1] \u7684\u6863\u6848\u5df2\u5f7b\u5e95\u5220\u9664\uff0c\u5176\u540d\u4e0b\u5ba2\u6237\u5df2\u8f6c\u5165\u516c\u6d77\u6c60\u3002")
                             .arg(m_targetUser.getUsername()));
    accept();
}
