#include "ManagerMainWindow.h"
#include "customerdetaildialog.h"
#include"userdetaildialog.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include<QMenu>
#include <fstream>

ManagerMainWindow::ManagerMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : BaseMainWindow(repo, user, parent)
{

    // 2. 默认选中第一个菜单（下属团队员工）
    if (m_leftMenu->count() > 0) {
        m_leftMenu->setCurrentRow(0);
    }
}

// =========================================================================
//  1. 菜单注入：纯文本菜单
// =========================================================================
void ManagerMainWindow::initRoleMenu()
{
    m_leftMenu->clear();
    m_leftMenu->addItems({
        "下属团队员工", // 下属团队员工
        "团队客户总览", // 团队客户总览
        "系统公海池"    // 系统公海池
    });
}

// =========================================================================
//  2. 菜单切换：多重视图切换
// =========================================================================
void ManagerMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear(); // 切换菜单时清空搜索框

    if (index == 0) {
        renderTeamUsers();
    }
    else if (index == 1) {
        m_displayedCustomers = m_repo->getCustomersByDepartment(m_currentUser.getDepartment());
        renderCustomers(m_displayedCustomers);
    }
    else if (index == 2) {
        m_displayedCustomers = m_repo->getHighSeasCustomers();
        renderCustomers(m_displayedCustomers);
    }
}

// =========================================================================
//  3. 部门级搜索隔离
// =========================================================================
void ManagerMainWindow::executeSearch(const QString& key)
{
    //如果没有搜索词，返回到当前行
    if (key.isEmpty()) {
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    // 如果当前行是显示员工（即第0行）的，那就执行查看员工的
    if (m_leftMenu->currentRow() == 0) {
        std::vector<User> filteredUsers;
        for (const auto& u : m_repo->getAllUsers()) {
        if (u.getDepartment() == m_currentUser.getDepartment() &&
                u.isActive() &&
                (u.getUsername().contains(key) || u.getUserId().contains(key))) {
                filteredUsers.push_back(u);
            }
        }
        m_teamUsers = filteredUsers;
        renderTeamUsers(m_teamUsers);
    }
    else {
        m_displayedCustomers = m_repo->searchCustomers(key, m_currentUser);
        renderCustomers(m_displayedCustomers);
    }
}

// =========================================================================
//  4. 双击行业务：规范化提示文本，剔除符号
// =========================================================================
void ManagerMainWindow::executeRowModification(int row)
{
    int currentMenuIndex = m_leftMenu->currentRow();

    if (currentMenuIndex == 0) {
        // =========================================================================
        // 选中了"下属团队员工"菜单（经理/管理员双击员工行）
        // =========================================================================
        if (row < 0 || row >= static_cast<int>(m_teamUsers.size())) return;

        // 1. 抓取目标员工 ID
        QString targetUserId = m_teamUsers[row].getUserId();

        // 2. 华丽地拉起员工大统一全功能详情舱
        // 传入当前登录人 m_currentUser，弹窗内部会自动判定他是 Manager 还是 Admin，从而开放离职移交特权
        UserDetailDialog userDlg(targetUserId, m_repo, m_currentUser, this);

        if (userDlg.exec() == QDialog::Accepted) {
            // 如果在弹窗里成功办理了离职移交，返回后原地刷新员工大表
            refreshDataByMenu(currentMenuIndex);
        }

    } else {
        // =========================================================================
        // 选中了"团队客户总览"或"系统公海池"菜单（保持你原有的高能逻辑）
        // =========================================================================
        if (row < 0 || row >= static_cast<int>(m_displayedCustomers.size())) return;

        // 2. 抓取目标客户 ID
        QString targetId = m_displayedCustomers[row].getId();

        // 3. 毫无阻碍地拉起统一客户大弹窗
        CustomerDetailDialog detailDlg(targetId, m_repo, m_currentUser, this);

        if (detailDlg.exec() == QDialog::Accepted) {
            // 如果经理/管理员在里面修改了基础数据、改派了销售、或者把人踢回了公海
            // 顺着当前高亮的左侧菜单原地刷新，让大表数据保持最新状态
            refreshDataByMenu(currentMenuIndex);
        }
    }
}

// =========================================================================
// 5. 核心算法：负载均衡指派器（纯文字清洗渲染）
// =========================================================================
bool ManagerMainWindow::allocateCustomerWithLoadBalancing(const QString& customerId)
{
    QDialog dlg(this);
    dlg.setWindowTitle("客户分配工具");
    dlg.setFixedWidth(380);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->addWidget(new QLabel("当前销售负载统计（负载均衡分配）：", &dlg));

    QComboBox* salesCombo = new QComboBox(&dlg);

    std::vector<User> allUsers = m_repo->getAllUsers();
    std::vector<Customer> allCustomers = m_repo->getAllCustomers();

    std::map<QString, int> loadMap;
    std::vector<User> departmentSales;

    for (const auto& u : allUsers) {
        if (u.getDepartment() == m_currentUser.getDepartment() && u.getRole() == UserRole::Sales && u.isActive()) {
            loadMap[u.getUserId()] = 0;
            departmentSales.push_back(u);
        }
    }

    for (const auto& c : allCustomers) {
        if (loadMap.find(c.getOwnerId()) != loadMap.end()) {
            loadMap[c.getOwnerId()]++;
        }
    }

    for (const auto& s : departmentSales) {
        int currentLoad = loadMap[s.getUserId()];
        QString itemText = QString::asprintf("%s (ID: %s) [Load: %d]",
                                             s.getUsername().toUtf8().constData(),
                                             s.getUserId().toUtf8().constData(),
                                             currentLoad);

        salesCombo->addItem(itemText, s.getUserId());
    }

    layout->addWidget(salesCombo);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QString targetSalesId = salesCombo->currentData().toString();
        if (targetSalesId.isEmpty()) return false;

        for (auto& c : allCustomers) {
            if (c.getId() == customerId) {
                c.setOwnerId(targetSalesId);
                m_repo->saveCustomer(c);

                // 洗入纯规范、无特殊符号的审计跟进日志
                FollowRecord auditLog(
                    "AUDIT_" + QString::number(QDateTime::currentMSecsSinceEpoch()),
                    customerId,
                    m_currentUser.getUserId(),
                    m_currentUser.getUsername(),
                    QString("[System Audit] Customer reassigned by manager via load balancing strategy."),
                    QDateTime::currentDateTime()
                    );
                m_repo->insertFollowRecord(auditLog);
                return true;
            }
        }
    }
    return false;
}

// =========================================================================
//️ 辅助数据渲染清洗函数
// =========================================================================

//渲染显示员工信息
void ManagerMainWindow::renderTeamUsers()
{
    //此处m_customerTable用作显示User
    m_customerTable->setRowCount(0); //清空所有旧行
    m_customerTable->setColumnCount(4);//设置4列
    m_customerTable->setHorizontalHeaderLabels({"员工ID", "名字", "部门", "账号状态"});

    std::vector<User> allUsers = m_repo->getAllUsers();
    m_teamUsers.clear();

    for (const auto& u : allUsers) {
        if (u.getDepartment() == m_currentUser.getDepartment() &&
                u.isActive() &&
                u.getRole() != UserRole::Admin) {
            m_teamUsers.push_back(u);
        }
    }

    for (size_t i = 0; i < m_teamUsers.size(); ++i) {
        m_customerTable->insertRow(i); //表格里新增一行
        //第一列 员工ID
        m_customerTable->setItem(i, 0, new QTableWidgetItem(m_teamUsers[i].getUserId()));
        //第二列 员工名字
        m_customerTable->setItem(i, 1, new QTableWidgetItem(m_teamUsers[i].getUsername()));
        //第三列 员工部门
        m_customerTable->setItem(i, 2, new QTableWidgetItem(m_teamUsers[i].getDepartment()));

        // 判断账号状态：Active 或 Terminated
        QString statusStr = m_teamUsers[i].isActive() ? "在职" : "离职";
        QTableWidgetItem* statusItem = new QTableWidgetItem(statusStr);

        //停用账号的文字设置为红色，区分异常账号
        if (!m_teamUsers[i].isActive()) {
            statusItem->setForeground(QBrush(Qt::red));
        }
        //第四列 账号状态
        m_customerTable->setItem(i, 3, statusItem);
    }
}

 void ManagerMainWindow::renderTeamUsers(const std ::vector<User>& indicatedUsers)
{
     //此处m_customerTable用作显示User
     m_customerTable->setRowCount(0); //清空所有旧行
     m_customerTable->setColumnCount(4);//设置4列
     m_customerTable->setHorizontalHeaderLabels({"员工ID", "名字", "部门", "账号状态"});

     for (size_t i = 0; i < indicatedUsers.size(); ++i) {
         m_customerTable->insertRow(i); //表格里新增一行
         //第一列 员工ID
         m_customerTable->setItem(i, 0, new QTableWidgetItem(indicatedUsers[i].getUserId()));
         //第二列 员工名字
         m_customerTable->setItem(i, 1, new QTableWidgetItem(indicatedUsers[i].getUsername()));
         //第三列 员工部门
         m_customerTable->setItem(i, 2, new QTableWidgetItem(indicatedUsers[i].getDepartment()));

         // 判断账号状态：Active 或 Terminated
         QString statusStr = indicatedUsers[i].isActive() ? "在职" : "离职";
         QTableWidgetItem* statusItem = new QTableWidgetItem(statusStr);

         //停用账号的文字设置为红色，区分异常账号
         if (!indicatedUsers[i].isActive()) {
             statusItem->setForeground(QBrush(Qt::red));
         }
         //第四列 账号状态
         m_customerTable->setItem(i, 3, statusItem);
     }
 }

//渲染显示客户信息，原理同renderTeamUsers
void ManagerMainWindow::renderCustomers(const std::vector<Customer>& customers)
{
    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({"客户ID", "客户名字", "电话", "状态"});

    for (size_t i = 0; i < customers.size(); ++i) {
        m_customerTable->insertRow(i);
        m_customerTable->setItem(i, 0, new QTableWidgetItem(customers[i].getId()));
        m_customerTable->setItem(i, 1, new QTableWidgetItem(customers[i].getName()));
        m_customerTable->setItem(i, 2, new QTableWidgetItem(customers[i].getPhone()));

        QString owner = customers[i].getOwnerId();
        QString ownerStr = owner.isEmpty() ? "公海池" : owner;
        QTableWidgetItem* ownerItem = new QTableWidgetItem(ownerStr);
        if (owner.isEmpty()) {
            ownerItem->setForeground(QBrush(Qt::yellow));
        }
        m_customerTable->setItem(i, 3, ownerItem);
    }
}
