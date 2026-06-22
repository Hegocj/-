#include "userdetaildialog.h"

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
    setWindowTitle(QStringLiteral("员工档案与资产交接"));
    resize(420, 320);

    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout();

    m_idEdit = new QLineEdit(this);
    m_nameEdit = new QLineEdit(this);
    m_deptEdit = new QLineEdit(this);
    m_roleEdit = new QLineEdit(this);
    m_statusEdit = new QLineEdit(this);

    m_idEdit->setReadOnly(true);
    m_nameEdit->setReadOnly(true);
    m_deptEdit->setReadOnly(true);
    m_roleEdit->setReadOnly(true);
    m_statusEdit->setReadOnly(true);

    formLayout->addRow(QStringLiteral("员工工号:"), m_idEdit);
    formLayout->addRow(QStringLiteral("员工姓名:"), m_nameEdit);
    formLayout->addRow(QStringLiteral("所属部门:"), m_deptEdit);
    formLayout->addRow(QStringLiteral("系统角色:"), m_roleEdit);
    formLayout->addRow(QStringLiteral("在职状态:"), m_statusEdit);
    mainLayout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    m_transferBtn = new QPushButton(QStringLiteral("办理离职并转移客户"), this);
    m_transferBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #d9534f; color: white; font-weight: bold; padding: 6px; }"
        "QPushButton:hover { background-color: #c9302c; }"));

    m_closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    m_closeBtn->setStyleSheet(QStringLiteral("padding: 6px 12px;"));

    m_deleteBtn = new QPushButton(QStringLiteral("彻底删除员工档案"), this);
    m_deleteBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #d9534f; color: white; font-weight: bold; padding: 6px; }"
        "QPushButton:hover { background-color: #c9302c; }"));

    if (m_currentUser.getRole() == UserRole::Sales) {
        m_transferBtn->setVisible(false);
    }
    if (m_currentUser.getRole() != UserRole::Admin) {
        m_deleteBtn->setVisible(false);
    }

    btnLayout->addWidget(m_transferBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(btnLayout);

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
        m_nameEdit->setText(QStringLiteral("未知员工"));
        m_statusEdit->setText(QStringLiteral("未找到该员工档案"));
        m_transferBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
        return;
    }

    m_idEdit->setText(m_targetUser.getUserId());
    m_nameEdit->setText(m_targetUser.getUsername());
    m_deptEdit->setText(m_targetUser.getDepartment());

    QString roleStr = QStringLiteral("销售");
    if (m_targetUser.getRole() == UserRole::Manager) {
        roleStr = QStringLiteral("经理");
    } else if (m_targetUser.getRole() == UserRole::Admin) {
        roleStr = QStringLiteral("管理员");
    }
    m_roleEdit->setText(roleStr);
    m_statusEdit->setText(m_targetUser.isActive() ? QStringLiteral("在职") : QStringLiteral("已离职"));
    m_transferBtn->setEnabled(m_targetUser.isActive() && m_targetUser.getRole() == UserRole::Sales);
}

void UserDetailDialog::onTransferButtonClicked()
{
    if (m_currentUser.getRole() == UserRole::Sales) {
        QMessageBox::critical(this, QStringLiteral("权限拦截"),
                              QStringLiteral("销售人员无权办理员工离职。"));
        return;
    }

    if (m_targetUser.getRole() != UserRole::Sales) {
        QMessageBox::warning(this, QStringLiteral("无法办理"),
                             QStringLiteral("当前仅支持对销售人员办理客户资产交接。"));
        return;
    }

    QStringList receiverItems;
    QStringList receiverIds;
    receiverItems << QStringLiteral("转入公海池");
    receiverIds << QString();

    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Sales &&
            user.isActive() &&
            user.getUserId() != m_targetUserId &&
            user.getDepartment() == m_targetUser.getDepartment()) {
            receiverItems << QStringLiteral("%1（%2）").arg(user.getUsername(), user.getUserId());
            receiverIds << user.getUserId();
        }
    }

    bool ok = false;
    const QString receiverText = QInputDialog::getItem(
        this,
        QStringLiteral("选择客户接收人"),
        QStringLiteral("请选择该员工名下客户的接收方式："),
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
        ? QStringLiteral("确定将员工 [%1] 标记为离职，并把其名下客户转入公海池吗？").arg(m_targetUser.getUsername())
        : QStringLiteral("确定将员工 [%1] 标记为离职，并把其名下客户转交给 [%2] 吗？").arg(m_targetUser.getUsername(), receiverText);

    if (QMessageBox::warning(this,
                             QStringLiteral("员工离职交接确认"),
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
        QMessageBox::critical(this, QStringLiteral("办理失败"),
                              QStringLiteral("离职状态或客户资产交接保存失败，请检查数据库状态。"));
        return;
    }

    QMessageBox::information(this, QStringLiteral("办理完成"),
                             QStringLiteral("员工 [%1] 已办理离职，客户资产已完成交接。").arg(m_targetUser.getUsername()));
    accept();
}

void UserDetailDialog::onDeleteButtonClicked()
{
    QMessageBox::information(this, QStringLiteral("暂不支持"),
                             QStringLiteral("当前仓储接口未提供员工物理删除功能，请使用离职冻结处理。"));
}
