/**
 * @file managermainwindow.cpp
 * @brief 经理主界面的业务实现。
 *
 * 研究定位：本文件实现部门经理的日常管理视角，重点围绕团队成员、部门客户、公海池
 * 客户和客户分配展开。它体现中层管理角色在系统中的权限边界。
 *
 * 主要职责：按菜单显示团队员工或客户列表；支持搜索部门范围内的员工/客户；打开
 * 员工详情和客户详情；在分配客户时统计销售当前客户数量，实现简单负载均衡提示。
 */
#include "ManagerMainWindow.h"
#include "customerdetaildialog.h"
#include "userdetaildialog.h"

#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidgetItem>
#include <QVBoxLayout>

ManagerMainWindow::ManagerMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : BaseMainWindow(repo, user, parent)
{
}

void ManagerMainWindow::initRoleMenu()
{
    m_leftMenu->clear();
    m_leftMenu->addItems({
        QStringLiteral("\u4e0b\u5c5e\u56e2\u961f\u5458\u5de5"),
        QStringLiteral("\u56e2\u961f\u5ba2\u6237\u603b\u89c8"),
        QStringLiteral("\u7cfb\u7edf\u516c\u6d77\u6c60")
    });
}

void ManagerMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear();

    if (index == 0) {
        renderTeamUsers();
    } else if (index == 1) {
        m_displayedCustomers = m_repo->getCustomersByDepartment(m_currentUser.getDepartment());
        renderCustomers(m_displayedCustomers);
    } else if (index == 2) {
        m_displayedCustomers = m_repo->getHighSeasCustomers();
        renderCustomers(m_displayedCustomers);
    }
}

void ManagerMainWindow::executeSearch(const QString& key)
{
    if (key.isEmpty()) {
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    if (m_leftMenu->currentRow() == 0) {
        std::vector<User> filteredUsers;
        for (const auto& user : m_repo->getAllUsers()) {
            if (user.getDepartment() == m_currentUser.getDepartment() &&
                user.getRole() != UserRole::Admin &&
                (user.getUsername().contains(key, Qt::CaseInsensitive) ||
                 user.getUserId().contains(key, Qt::CaseInsensitive))) {
                filteredUsers.push_back(user);
            }
        }
        m_teamUsers = filteredUsers;
        renderTeamUsers(m_teamUsers);
        return;
    }

    m_displayedCustomers = m_repo->searchCustomers(key, m_currentUser);
    renderCustomers(m_displayedCustomers);
}

void ManagerMainWindow::executeRowModification(int row)
{
    const int currentMenuIndex = m_leftMenu->currentRow();

    if (currentMenuIndex == 0) {
        if (row < 0 || row >= static_cast<int>(m_teamUsers.size())) {
            return;
        }

        UserDetailDialog userDlg(m_teamUsers[row].getUserId(), m_repo, m_currentUser, this);
        if (userDlg.exec() == QDialog::Accepted) {
            refreshDataByMenu(currentMenuIndex);
        }
        return;
    }

    if (row < 0 || row >= static_cast<int>(m_displayedCustomers.size())) {
        return;
    }

    CustomerDetailDialog detailDlg(m_displayedCustomers[row].getId(), m_repo, m_currentUser, this);
    if (detailDlg.exec() == QDialog::Accepted) {
        refreshDataByMenu(currentMenuIndex);
    }
}

bool ManagerMainWindow::allocateCustomerWithLoadBalancing(const QString& customerId)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u5ba2\u6237\u5206\u914d"));
    dialog.setFixedWidth(380);

    auto* layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(QStringLiteral("\u9009\u62e9\u8981\u5206\u914d\u7684\u9500\u552e\uff1a"), &dialog));

    auto* salesCombo = new QComboBox(&dialog);
    const std::vector<User> allUsers = m_repo->getAllUsers();
    const std::vector<Customer> allCustomers = m_repo->getAllCustomers();

    std::map<QString, int> loadMap;
    std::vector<User> departmentSales;
    for (const auto& user : allUsers) {
        if (user.getDepartment() == m_currentUser.getDepartment() &&
            user.getRole() == UserRole::Sales &&
            user.isActive()) {
            loadMap[user.getUserId()] = 0;
            departmentSales.push_back(user);
        }
    }

    for (const auto& customer : allCustomers) {
        if (loadMap.find(customer.getOwnerId()) != loadMap.end()) {
            ++loadMap[customer.getOwnerId()];
        }
    }

    for (const auto& sales : departmentSales) {
        salesCombo->addItem(QStringLiteral("%1 (ID: %2) [\u5f53\u524d\u5ba2\u6237: %3]")
                                .arg(sales.getUsername(), sales.getUserId())
                                .arg(loadMap[sales.getUserId()]),
                            sales.getUserId());
    }
    layout->addWidget(salesCombo);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    const QString targetSalesId = salesCombo->currentData().toString();
    if (targetSalesId.isEmpty()) {
        return false;
    }

    for (auto customer : allCustomers) {
        if (customer.getId() == customerId) {
            customer.setOwnerId(targetSalesId);
            if (!m_repo->saveCustomer(customer)) {
                return false;
            }

            FollowRecord auditLog(
                QStringLiteral("AUDIT_%1").arg(QDateTime::currentMSecsSinceEpoch()),
                customerId,
                m_currentUser.getUserId(),
                m_currentUser.getUsername(),
                QStringLiteral("Customer reassigned by manager."),
                QDateTime::currentDateTime()
            );
            m_repo->insertFollowRecord(auditLog);
            return true;
        }
    }

    return false;
}

void ManagerMainWindow::renderTeamUsers()
{
    m_teamUsers.clear();
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getDepartment() == m_currentUser.getDepartment() &&
            user.getRole() != UserRole::Admin) {
            m_teamUsers.push_back(user);
        }
    }
    renderTeamUsers(m_teamUsers);
}

void ManagerMainWindow::renderTeamUsers(const std::vector<User>& indicatedUsers)
{
    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5458\u5de5ID"),
        QStringLiteral("\u59d3\u540d"),
        QStringLiteral("\u90e8\u95e8"),
        QStringLiteral("\u8d26\u53f7\u72b6\u6001")
    });

    for (int row = 0; row < static_cast<int>(indicatedUsers.size()); ++row) {
        const User& user = indicatedUsers.at(static_cast<size_t>(row));
        m_customerTable->insertRow(row);
        m_customerTable->setItem(row, 0, new QTableWidgetItem(user.getUserId()));
        m_customerTable->setItem(row, 1, new QTableWidgetItem(user.getUsername()));
        m_customerTable->setItem(row, 2, new QTableWidgetItem(user.getDepartment()));

        auto* statusItem = new QTableWidgetItem(user.isActive()
                                                    ? QStringLiteral("\u5728\u804c")
                                                    : QStringLiteral("\u79bb\u804c"));
        if (!user.isActive()) {
            statusItem->setForeground(QBrush(Qt::red));
        }
        m_customerTable->setItem(row, 3, statusItem);
    }
}

void ManagerMainWindow::renderCustomers(const std::vector<Customer>& customers)
{
    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(5);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5ba2\u6237ID"),
        QStringLiteral("\u5ba2\u6237\u59d3\u540d"),
        QStringLiteral("\u8054\u7cfb\u7535\u8bdd"),
        QStringLiteral("\u5ba2\u6237\u7b49\u7ea7"),
        QStringLiteral("\u8d1f\u8d23\u9500\u552e")
    });

    for (int row = 0; row < static_cast<int>(customers.size()); ++row) {
        const Customer& customer = customers.at(static_cast<size_t>(row));
        m_customerTable->insertRow(row);
        const bool isVip = customer.getLevel().compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0;
        auto* idItem = new QTableWidgetItem(customer.getId());
        auto* nameItem = new QTableWidgetItem(isVip
                                                  ? QStringLiteral("[VIP] %1").arg(customer.getName())
                                                  : customer.getName());
        auto* phoneItem = new QTableWidgetItem(customer.getPhone());
        auto* levelItem = new QTableWidgetItem(isVip ? QStringLiteral("VIP") : QStringLiteral("\u666e\u901a"));

        const QString owner = customer.getOwnerId();
        auto* ownerItem = new QTableWidgetItem(owner.isEmpty()
                                                   ? QStringLiteral("\u516c\u6d77\u6c60")
                                                   : owner);
        if (owner.isEmpty()) {
            ownerItem->setForeground(QBrush(Qt::darkYellow));
        }
        if (isVip) {
            const QBrush vipBrush(QColor(255, 245, 210));
            for (auto* item : {idItem, nameItem, phoneItem, levelItem, ownerItem}) {
                item->setBackground(vipBrush);
                item->setForeground(QBrush(QColor(128, 78, 0)));
            }
            levelItem->setToolTip(QStringLiteral("VIP\u5ba2\u6237\uff0c\u5df2\u7f6e\u9876\u663e\u793a"));
        }
        m_customerTable->setItem(row, 0, idItem);
        m_customerTable->setItem(row, 1, nameItem);
        m_customerTable->setItem(row, 2, phoneItem);
        m_customerTable->setItem(row, 3, levelItem);
        m_customerTable->setItem(row, 4, ownerItem);
    }
}
