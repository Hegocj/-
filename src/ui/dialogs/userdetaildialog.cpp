/**
 * @file userdetaildialog.cpp
 * @brief 员工档案与账号维护弹窗的业务实现。
 *
 * 研究定位：本文件实现员工账号生命周期管理，包括基本资料维护、角色调整、密码修改、
 * 离职交接和删除。它保障客户资产不会因员工离职或删除而失去归属处理。
 *
 * 主要职责：根据当前操作者角色控制编辑权限；保存员工资料；为离职销售选择客户接收人
 * 或转入公海池；管理员删除员工时自动释放其名下客户。
 *
 * 编码说明：本文件中形如 "\u4e2d\u6587" 的内容是 Unicode 转义字符串，
 * 编译运行后会显示为中文，用来避免不同编辑器编码设置导致界面文字乱码。
 */
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
    setWindowTitle(QStringLiteral("\u5458\u5de5\u6863\u6848\u4e0e\u8d26\u53f7\u7ef4\u62a4")); // 中文: 员工档案与账号维护
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
    m_roleCombo->addItem(QStringLiteral("\u9500\u552e"), static_cast<int>(UserRole::Sales)); // 中文: 销售
    m_roleCombo->addItem(QStringLiteral("\u7ecf\u7406"), static_cast<int>(UserRole::Manager)); // 中文: 经理
    m_roleCombo->addItem(QStringLiteral("\u7ba1\u7406\u5458"), static_cast<int>(UserRole::Admin)); // 中文: 管理员
    m_statusEdit->setReadOnly(true);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QStringLiteral("\u7559\u7a7a\u8868\u793a\u4e0d\u4fee\u6539\u5bc6\u7801")); // 中文: 留空表示不修改密码

    formLayout->addRow(QStringLiteral("\u5458\u5de5\u5de5\u53f7:"), m_idEdit); // 中文: 员工工号:
    formLayout->addRow(QStringLiteral("\u767b\u5f55\u7528\u6237\u540d:"), m_nameEdit); // 中文: 登录用户名:
    formLayout->addRow(QStringLiteral("\u6240\u5c5e\u90e8\u95e8:"), m_deptEdit); // 中文: 所属部门:
    formLayout->addRow(QStringLiteral("\u7cfb\u7edf\u89d2\u8272:"), m_roleCombo); // 中文: 系统角色:
    formLayout->addRow(QStringLiteral("\u8d26\u53f7\u72b6\u6001:"), m_statusEdit); // 中文: 账号状态:
    formLayout->addRow(QStringLiteral("\u65b0\u5bc6\u7801:"), m_passwordEdit); // 中文: 新密码:
    mainLayout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("\u4fdd\u5b58\u4fee\u6539"), this); // 中文: 保存修改
    m_transferBtn = new QPushButton(QStringLiteral("\u529e\u7406\u79bb\u804c\u5e76\u8f6c\u79fb\u5ba2\u6237"), this); // 中文: 办理离职并转移客户
    m_deleteBtn = new QPushButton(QStringLiteral("\u5f7b\u5e95\u5220\u9664\u5458\u5de5\u6863\u6848"), this); // 中文: 彻底删除员工档案
    m_closeBtn = new QPushButton(QStringLiteral("\u5173\u95ed"), this); // 中文: 关闭

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
        m_nameEdit->setText(QStringLiteral("\u672a\u77e5\u5458\u5de5")); // 中文: 未知员工
        m_statusEdit->setText(QStringLiteral("\u672a\u627e\u5230\u8be5\u5458\u5de5\u6863\u6848")); // 中文: 未找到该员工档案
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
                              ? QStringLiteral("\u5728\u804c") // 中文: 在职
                              : QStringLiteral("\u79bb\u804c")); // 中文: 离职

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
                             QStringLiteral("\u4fdd\u5b58\u5931\u8d25"), // 中文: 保存失败
                             QStringLiteral("\u7528\u6237\u540d\u548c\u90e8\u95e8\u4e0d\u80fd\u4e3a\u7a7a\u3002")); // 中文: 用户名和部门不能为空。
        return;
    }

    User updated = m_targetUser;
    updated.setUsername(m_nameEdit->text().trimmed());
    updated.setDepartment(m_deptEdit->text().trimmed());
    updated.setRole(static_cast<UserRole>(m_roleCombo->currentData().toInt()));
    updated.setActive(m_targetUser.isActive());

    if (!m_repo->saveUser(updated, m_passwordEdit->text())) {
        QMessageBox::critical(this,
                              QStringLiteral("\u4fdd\u5b58\u5931\u8d25"), // 中文: 保存失败
                              QStringLiteral("\u5458\u5de5\u4fe1\u606f\u4fdd\u5b58\u5931\u8d25\u3002")); // 中文: 员工信息保存失败。
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u4fdd\u5b58\u6210\u529f"), // 中文: 保存成功
                             QStringLiteral("\u5458\u5de5\u4fe1\u606f\u5df2\u66f4\u65b0\u3002")); // 中文: 员工信息已更新。
    accept();
}

void UserDetailDialog::onTransferButtonClicked()
{
    if (m_currentUser.getRole() == UserRole::Sales) {
        QMessageBox::critical(this,
                              QStringLiteral("\u6743\u9650\u62e6\u622a"), // 中文: 权限拦截
                              QStringLiteral("\u9500\u552e\u4eba\u5458\u65e0\u6743\u529e\u7406\u5458\u5de5\u79bb\u804c\u3002")); // 中文: 销售人员无权办理员工离职。
        return;
    }

    if (m_targetUser.getRole() != UserRole::Sales) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u6cd5\u529e\u7406"), // 中文: 无法办理
                             QStringLiteral("\u5f53\u524d\u4ec5\u652f\u6301\u5bf9\u9500\u552e\u4eba\u5458\u529e\u7406\u5ba2\u6237\u8d44\u4ea7\u4ea4\u63a5\u3002")); // 中文: 当前仅支持对销售人员办理客户资产交接。
        return;
    }

    QStringList receiverItems;
    QStringList receiverIds;
    receiverItems << QStringLiteral("\u8f6c\u5165\u516c\u6d77\u6c60"); // 中文: 转入公海池
    receiverIds << QString();

    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Sales &&
            user.isActive() &&
            user.getUserId() != m_targetUserId &&
            user.getDepartment() == m_targetUser.getDepartment()) {
            receiverItems << QStringLiteral("%1\uff08%2\uff09").arg(user.getUsername(), user.getUserId()); // 中文: %1（%2）
            receiverIds << user.getUserId();
        }
    }

    bool ok = false;
    const QString receiverText = QInputDialog::getItem(
        this,
        QStringLiteral("\u9009\u62e9\u5ba2\u6237\u63a5\u6536\u4eba"), // 中文: 选择客户接收人
        QStringLiteral("\u8bf7\u9009\u62e9\u8be5\u5458\u5de5\u540d\u4e0b\u5ba2\u6237\u7684\u63a5\u6536\u65b9\u5f0f\uff1a"), // 中文: 请选择该员工名下客户的接收方式：
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
        ? QStringLiteral("\u786e\u5b9a\u5c06\u5458\u5de5 [%1] \u6807\u8bb0\u4e3a\u79bb\u804c\uff0c\u5e76\u628a\u5176\u540d\u4e0b\u5ba2\u6237\u8f6c\u5165\u516c\u6d77\u6c60\u5417\uff1f").arg(m_targetUser.getUsername()) // 中文: 确定将员工 [%1] 标记为离职，并把其名下客户转入公海池吗？
        : QStringLiteral("\u786e\u5b9a\u5c06\u5458\u5de5 [%1] \u6807\u8bb0\u4e3a\u79bb\u804c\uff0c\u5e76\u628a\u5176\u540d\u4e0b\u5ba2\u6237\u8f6c\u4ea4\u7ed9 [%2] \u5417\uff1f").arg(m_targetUser.getUsername(), receiverText); // 中文: 确定将员工 [%1] 标记为离职，并把其名下客户转交给 [%2] 吗？

    if (QMessageBox::warning(this,
                             QStringLiteral("\u5458\u5de5\u79bb\u804c\u4ea4\u63a5\u786e\u8ba4"), // 中文: 员工离职交接确认
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
                              QStringLiteral("\u529e\u7406\u5931\u8d25"), // 中文: 办理失败
                              QStringLiteral("\u79bb\u804c\u72b6\u6001\u6216\u5ba2\u6237\u8d44\u4ea7\u4ea4\u63a5\u4fdd\u5b58\u5931\u8d25\u3002")); // 中文: 离职状态或客户资产交接保存失败。
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u529e\u7406\u5b8c\u6210"), // 中文: 办理完成
                             QStringLiteral("\u5458\u5de5 [%1] \u5df2\u529e\u7406\u79bb\u804c\uff0c\u5ba2\u6237\u8d44\u4ea7\u5df2\u5b8c\u6210\u4ea4\u63a5\u3002").arg(m_targetUser.getUsername())); // 中文: 员工 [%1] 已办理离职，客户资产已完成交接。
    accept();
}

void UserDetailDialog::onDeleteButtonClicked()
{
    if (m_currentUser.getRole() != UserRole::Admin) {
        QMessageBox::warning(this,
                             QStringLiteral("\u6743\u9650\u62e6\u622a"), // 中文: 权限拦截
                             QStringLiteral("\u53ea\u6709\u7ba1\u7406\u5458\u624d\u80fd\u5220\u9664\u5458\u5de5\u6863\u6848\u3002")); // 中文: 只有管理员才能删除员工档案。
        return;
    }

    if (m_targetUser.getUserId() == m_currentUser.getUserId()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u6cd5\u5220\u9664"), // 中文: 无法删除
                             QStringLiteral("\u4e0d\u80fd\u5220\u9664\u81ea\u5df1\u7684\u8d26\u53f7\u3002")); // 中文: 不能删除自己的账号。
        return;
    }

    const auto confirmText = QStringLiteral(
        "\u786e\u5b9a\u8981\u5f7b\u5e95\u5220\u9664\u5458\u5de5 [%1] \u7684\u6863\u6848\u5417\uff1f\n\n" // 中文: 确定要彻底删除员工 [%1] 的档案吗？
        "\u6ce8\u610f\uff1a\u8be5\u5458\u5de5\u540d\u4e0b\u7684\u6240\u6709\u5ba2\u6237\u5c06\u81ea\u52a8\u8f6c\u5165\u516c\u6d77\u6c60\uff0c\u8be5\u64cd\u4f5c\u4e0d\u53ef\u6062\u590d\uff01" // 中文: 注意：该员工名下的所有客户将自动转入公海池，该操作不可恢复！
    ).arg(m_targetUser.getUsername());

    if (QMessageBox::warning(this,
                             QStringLiteral("\u786e\u8ba4\u5220\u9664"), // 中文: 确认删除
                             confirmText,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (!m_repo->deleteUser(m_targetUser.getUserId())) {
        QMessageBox::critical(this,
                             QStringLiteral("\u5220\u9664\u5931\u8d25"), // 中文: 删除失败
                             QStringLiteral("\u5458\u5de5\u6863\u6848\u5220\u9664\u5931\u8d25\u3002")); // 中文: 员工档案删除失败。
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u5220\u9664\u6210\u529f"), // 中文: 删除成功
                             QStringLiteral("\u5458\u5de5 [%1] \u7684\u6863\u6848\u5df2\u5f7b\u5e95\u5220\u9664\uff0c\u5176\u540d\u4e0b\u5ba2\u6237\u5df2\u8f6c\u5165\u516c\u6d77\u6c60\u3002") // 中文: 员工 [%1] 的档案已彻底删除，其名下客户已转入公海池。
                             .arg(m_targetUser.getUsername()));
    accept();
}
