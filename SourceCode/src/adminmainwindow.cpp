/**
 * @file adminmainwindow.cpp
 * @brief 管理员主界面的业务实现。
 *
 * 研究定位：本文件实现管理员视角下的系统总控台，是客户信息管理系统中权限最高的
 * 操作入口。它将客户、部门、员工账号等资源统一呈现在主窗口中，体现系统的集中管理
 * 特征。
 *
 * 主要职责：渲染全部客户、公海池客户和部门组织树；支持新增客户、销售、经理账号；
 * 支持查看账号密码、分配部门经理、打开客户详情和员工详情弹窗。其协作对象主要包括
 * CustomerDetailDialog、UserDetailDialog 与 ICustomerRepository。
 *
 * 编码说明：本文件中形如 "\u4e2d\u6587" 的内容是 Unicode 转义字符串，
 * 编译运行后会显示为中文，用来避免不同编辑器编码设置导致界面文字乱码。
 */
#include "AdminMainWindow.h"
#include "customerdetaildialog.h"
#include "userdetaildialog.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <map>

static QTreeWidget* s_orgTree = nullptr;

AdminMainWindow::AdminMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget* parent)
    : BaseMainWindow(repo, user, parent)
{
    if (m_rightContainer && m_rightContainer->layout()) {
        // QStringLiteral 中的 \uXXXX 是 Unicode 转义写法，用来表示中文字符。
        // 例如 "\u6dfb\u52a0\u5ba2\u6237" 运行时会显示为“添加客户”，这样可以避免源码编码不一致导致中文乱码。
        m_addCustomerBtn = new QPushButton(QStringLiteral("\u6dfb\u52a0\u5ba2\u6237"), m_rightContainer); // 中文: 添加客户
        m_addSalesBtn = new QPushButton(QStringLiteral("\u6dfb\u52a0\u9500\u552e"), m_rightContainer); // 中文: 添加销售
        m_addManagerBtn = new QPushButton(QStringLiteral("\u6dfb\u52a0\u7ecf\u7406"), m_rightContainer); // 中文: 添加经理
        m_generateDataBtn = new QPushButton(QStringLiteral("\u751f\u6210\u6d4b\u8bd5\u6570\u636e"), m_rightContainer); // 中文: 生成测试数据
        m_accountPasswordBtn = new QPushButton(QStringLiteral("\u67e5\u770b\u7ba1\u7406\u5458/\u7ecf\u7406/\u9500\u552e\u8d26\u53f7\u5bc6\u7801"), m_rightContainer); // 中文: 查看管理员/经理/销售账号密码

        m_rightContainer->layout()->addWidget(m_addCustomerBtn);
        m_rightContainer->layout()->addWidget(m_addSalesBtn);
        m_rightContainer->layout()->addWidget(m_addManagerBtn);
        m_rightContainer->layout()->addWidget(m_generateDataBtn);
        m_rightContainer->layout()->addWidget(m_accountPasswordBtn);

        connect(m_addCustomerBtn, &QPushButton::clicked, this, &AdminMainWindow::showAddCustomerDialog);
        connect(m_addSalesBtn, &QPushButton::clicked, this, &AdminMainWindow::showAddSalesDialog);
        connect(m_addManagerBtn, &QPushButton::clicked, this, &AdminMainWindow::showAddManagerDialog);
        connect(m_generateDataBtn, &QPushButton::clicked, this, &AdminMainWindow::generateRandomData);
        connect(m_accountPasswordBtn, &QPushButton::clicked, this, &AdminMainWindow::showAccountPasswordDialog);
    }

    if (m_customerTable && m_customerTable->parentWidget()) {
        s_orgTree = new QTreeWidget(m_customerTable->parentWidget());
        s_orgTree->setColumnCount(3);
        s_orgTree->setHeaderLabels({
            QStringLiteral("\u90e8\u95e8"), // 中文: 部门
            QStringLiteral("\u5458\u5de5ID"), // 中文: 员工ID
            QStringLiteral("\u804c\u6743") // 中文: 职权
        });
        s_orgTree->setColumnWidth(0, 300);
        s_orgTree->setColumnWidth(1, 120);
        s_orgTree->setColumnWidth(2, 100);
        s_orgTree->header()->setStretchLastSection(false);
        s_orgTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        s_orgTree->hide();

        connect(s_orgTree, &QTreeWidget::itemDoubleClicked,
                this, &AdminMainWindow::onTreeItemDoubleClicked);

        if (m_customerTable->parentWidget()->layout()) {
            m_customerTable->parentWidget()->layout()->addWidget(s_orgTree);
        }
    }

    if (m_leftMenu->count() > 0) {
        m_leftMenu->setCurrentRow(0);
    }
}

void AdminMainWindow::initRoleMenu()
{
    m_leftMenu->clear();
    m_leftMenu->addItems({
        QStringLiteral("\u5168\u90e8\u5ba2\u6237"), // 中文: 全部客户
        QStringLiteral("\u5168\u90e8\u90e8\u95e8"), // 中文: 全部部门
        QStringLiteral("\u516c\u6d77\u6c60") // 中文: 公海池
    });
}

void AdminMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear();

    if (index == 1) {
        if (m_customerTable) {
            m_customerTable->hide();
        }
        if (s_orgTree) {
            s_orgTree->show();
            renderDepartmentTree();
        }
        return;
    }

    if (s_orgTree) {
        s_orgTree->hide();
    }
    if (m_customerTable) {
        m_customerTable->show();
        m_globalCustomersCache = (index == 0) ? m_repo->getAllCustomers() : m_repo->getHighSeasCustomers();
        renderGlobalCustomers(m_globalCustomersCache);
    }
}

void AdminMainWindow::renderDepartmentTree()
{
    if (!s_orgTree) {
        return;
    }

    s_orgTree->clear();
    const std::vector<User> allUsers = m_repo->getAllUsers();
    QStringList managerDepartments;

    for (const auto& user : allUsers) {
        if (user.getRole() != UserRole::Manager) {
            continue;
        }

        managerDepartments << user.getDepartment();

        auto* managerItem = new QTreeWidgetItem(s_orgTree);
        managerItem->setText(0, QStringLiteral("%1 [\u7ecf\u7406: %2]").arg(user.getDepartment(), user.getUsername())); // 中文: %1 [经理: %2]
        managerItem->setText(1, user.getUserId());
        managerItem->setText(2, user.isActive()
                                 ? QStringLiteral("\u5728\u804c\u7ecf\u7406") // 中文: 在职经理
                                 : QStringLiteral("\u79bb\u804c\u7ecf\u7406")); // 中文: 离职经理
        managerItem->setData(0, Qt::UserRole, QStringLiteral("manager"));
        managerItem->setData(0, Qt::UserRole + 1, user.getDepartment());

        for (const auto& sales : allUsers) {
            if (sales.getDepartment() == user.getDepartment() &&
                sales.getRole() == UserRole::Sales) {
                auto* salesItem = new QTreeWidgetItem(managerItem);
                salesItem->setText(0, QStringLiteral("  \u9500\u552e %1").arg(sales.getUsername())); // 中文: 销售 %1
                salesItem->setText(1, sales.getUserId());
                salesItem->setText(2, sales.isActive()
                                       ? QStringLiteral("\u5728\u804c") // 中文: 在职
                                       : QStringLiteral("\u79bb\u804c")); // 中文: 离职
                salesItem->setData(0, Qt::UserRole, QStringLiteral("sales"));
                salesItem->setData(0, Qt::UserRole + 1, sales.getDepartment());
            }
        }
    }

    for (const auto& sales : allUsers) {
        if (sales.getRole() != UserRole::Sales ||
            managerDepartments.contains(sales.getDepartment())) {
            continue;
        }

        auto* departmentItem = new QTreeWidgetItem(s_orgTree);
        departmentItem->setText(0, QStringLiteral("%1 [\u65e0\u7ecf\u7406]").arg(sales.getDepartment())); // 中文: %1 [无经理]
        departmentItem->setText(1, QString());
        departmentItem->setText(2, QStringLiteral("\u53cc\u51fb\u8bbe\u7f6e\u6240\u5c5e\u7ecf\u7406")); // 中文: 双击设置所属经理
        departmentItem->setData(0, Qt::UserRole, QStringLiteral("department"));
        departmentItem->setData(0, Qt::UserRole + 1, sales.getDepartment());

        auto* salesItem = new QTreeWidgetItem(departmentItem);
        salesItem->setText(0, QStringLiteral("  \u9500\u552e %1").arg(sales.getUsername())); // 中文: 销售 %1
        salesItem->setText(1, sales.getUserId());
        salesItem->setText(2, sales.isActive()
                               ? QStringLiteral("\u5728\u804c") // 中文: 在职
                               : QStringLiteral("\u79bb\u804c")); // 中文: 离职
        salesItem->setData(0, Qt::UserRole, QStringLiteral("sales"));
        salesItem->setData(0, Qt::UserRole + 1, sales.getDepartment());
    }

    s_orgTree->collapseAll();
}

void AdminMainWindow::executeSearch(const QString& key)
{
    if (key.isEmpty()) {
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    const int currentMenuIndex = m_leftMenu->currentRow();
    if (currentMenuIndex == 1) {
        if (!s_orgTree) {
            return;
        }
        for (int i = 0; i < s_orgTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* parent = s_orgTree->topLevelItem(i);
            const bool parentMatch = parent->text(0).contains(key) || parent->text(1).contains(key);

            int visibleChildren = 0;
            for (int j = 0; j < parent->childCount(); ++j) {
                QTreeWidgetItem* child = parent->child(j);
                const bool childMatch = child->text(0).contains(key) || child->text(1).contains(key);
                child->setHidden(!childMatch);
                if (childMatch) {
                    ++visibleChildren;
                }
            }

            parent->setHidden(!(parentMatch || visibleChildren > 0));
            parent->setExpanded(parentMatch || visibleChildren > 0);
        }
        return;
    }

    const std::vector<Customer> source = (currentMenuIndex == 0) ? m_repo->getAllCustomers() : m_repo->getHighSeasCustomers();
    std::vector<Customer> filtered;
    for (const auto& customer : source) {
        if (customer.getName().contains(key) ||
            customer.getId().contains(key) ||
            customer.getPhone().contains(key)) {
            filtered.push_back(customer);
        }
    }
    renderGlobalCustomers(filtered);
}

void AdminMainWindow::executeRowModification(int row)
{
    if (m_leftMenu->currentRow() != 0 && m_leftMenu->currentRow() != 2) {
        return;
    }
    if (row < 0 || row >= static_cast<int>(m_globalCustomersCache.size())) {
        return;
    }

    CustomerDetailDialog detailDlg(m_globalCustomersCache[row].getId(), m_repo, m_currentUser, this);
    if (detailDlg.exec() == QDialog::Accepted) {
        refreshDataByMenu(m_leftMenu->currentRow());
    }
}

void AdminMainWindow::renderGlobalCustomers(const std::vector<Customer>& customers)
{
    m_globalCustomersCache = customers;
    std::map<QString, QString> salesNameById;
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Sales) {
            salesNameById[user.getUserId()] = user.getUsername();
        }
    }

    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(5);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5ba2\u6237ID"), // 中文: 客户ID
        QStringLiteral("\u5ba2\u6237\u59d3\u540d"), // 中文: 客户姓名
        QStringLiteral("\u8054\u7cfb\u7535\u8bdd"), // 中文: 联系电话
        QStringLiteral("\u5ba2\u6237\u7b49\u7ea7"), // 中文: 客户等级
        QStringLiteral("\u5f53\u524d\u8d1f\u8d23\u4eba") // 中文: 当前负责人
    });

    for (int row = 0; row < static_cast<int>(customers.size()); ++row) {
        const auto& customer = customers.at(static_cast<size_t>(row));
        m_customerTable->insertRow(row);
        const bool isVip = customer.getLevel().compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0;
        auto* idItem = new QTableWidgetItem(customer.getId());
        auto* nameItem = new QTableWidgetItem(isVip
                                                  ? QStringLiteral("[VIP] %1").arg(customer.getName())
                                                  : customer.getName());
        auto* phoneItem = new QTableWidgetItem(customer.getPhone());
        auto* levelItem = new QTableWidgetItem(isVip ? QStringLiteral("VIP") : QStringLiteral("\u666e\u901a")); // 中文: 普通
        const QString ownerId = customer.getOwnerId();
        const QString ownerText = salesNameById.find(ownerId) == salesNameById.end()
                                      ? ownerId
                                      : salesNameById.at(ownerId);
        auto* ownerItem = new QTableWidgetItem(customer.getOwnerId().isEmpty()
                                                   ? QStringLiteral("\u516c\u6d77\u6c60") // 中文: 公海池
                                                   : ownerText);
        if (isVip) {
            const QBrush vipBrush(QColor(255, 245, 210));
            for (auto* item : {idItem, nameItem, phoneItem, levelItem, ownerItem}) {
                item->setBackground(vipBrush);
                item->setForeground(QBrush(QColor(128, 78, 0)));
            }
            levelItem->setToolTip(QStringLiteral("VIP\u5ba2\u6237\uff0c\u5df2\u7f6e\u9876\u663e\u793a")); // 中文: VIP客户，已置顶显示
        }
        m_customerTable->setItem(row, 0, idItem);
        m_customerTable->setItem(row, 1, nameItem);
        m_customerTable->setItem(row, 2, phoneItem);
        m_customerTable->setItem(row, 3, levelItem);
        m_customerTable->setItem(row, 4, ownerItem);
    }
}

void AdminMainWindow::onTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item) {
        return;
    }

    const QString itemType = item->data(0, Qt::UserRole).toString();
    if (itemType == QStringLiteral("department")) {
        showDepartmentManagerDialog(item->data(0, Qt::UserRole + 1).toString());
        return;
    }

    const QString userId = item->text(1).trimmed();
    if (userId.isEmpty()) {
        return;
    }

    UserDetailDialog userDlg(userId, m_repo, m_currentUser, this);
    if (userDlg.exec() == QDialog::Accepted) {
        renderDepartmentTree();
    }
}

void AdminMainWindow::showDepartmentManagerDialog(const QString& department)
{
    const QString trimmedDepartment = department.trimmed();
    if (trimmedDepartment.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u6cd5\u8bbe\u7f6e"), // 中文: 无法设置
                             QStringLiteral("\u90e8\u95e8\u540d\u4e0d\u80fd\u4e3a\u7a7a\u3002")); // 中文: 部门名不能为空。
        return;
    }

    QStringList managerItems;
    QStringList managerIds;
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Manager && user.isActive()) {
            managerItems << QStringLiteral("%1\uff08%2\uff0c%3\uff09") // 中文: %1（%2，%3）
                                .arg(user.getUsername(), user.getUserId(), user.getDepartment());
            managerIds << user.getUserId();
        }
    }

    if (managerItems.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u65e0\u53ef\u7528\u7ecf\u7406"), // 中文: 无可用经理
                             QStringLiteral("\u5f53\u524d\u6ca1\u6709\u5728\u804c\u7ecf\u7406\u8d26\u53f7\uff0c\u8bf7\u5148\u6dfb\u52a0\u7ecf\u7406\u3002")); // 中文: 当前没有在职经理账号，请先添加经理。
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u8bbe\u7f6e\u90e8\u95e8\u6240\u5c5e\u7ecf\u7406")); // 中文: 设置部门所属经理
    dialog.resize(460, 180);

    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* managerCombo = new QComboBox(&dialog);
    auto* managerIdEdit = new QLineEdit(&dialog);

    managerCombo->addItems(managerItems);
    managerIdEdit->setPlaceholderText(QStringLiteral("\u4e5f\u53ef\u76f4\u63a5\u8f93\u5165\u7ecf\u7406ID")); // 中文: 也可直接输入经理ID
    form->addRow(QStringLiteral("\u5f53\u524d\u90e8\u95e8:"), new QLabel(trimmedDepartment, &dialog)); // 中文: 当前部门:
    form->addRow(QStringLiteral("\u9009\u62e9\u7ecf\u7406:"), managerCombo); // 中文: 选择经理:
    form->addRow(QStringLiteral("\u7ecf\u7406ID:"), managerIdEdit); // 中文: 经理ID:
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    User targetManager;
    bool managerFound = false;
    const QString inputManagerId = managerIdEdit->text().trimmed();
    if (!inputManagerId.isEmpty()) {
        for (const auto& user : m_repo->getAllUsers()) {
            if ((user.getUserId() == inputManagerId || user.getUsername() == inputManagerId) &&
                user.getRole() == UserRole::Manager &&
                user.isActive()) {
                managerFound = true;
                targetManager = user;
                break;
            }
        }

        if (!managerFound) {
            QMessageBox::warning(this,
                                 QStringLiteral("\u7ecf\u7406\u4e0d\u5b58\u5728"), // 中文: 经理不存在
                                 QStringLiteral("\u672a\u627e\u5230ID/\u7528\u6237\u540d\u4e3a [%1] \u7684\u5728\u804c\u7ecf\u7406\u8d26\u53f7\u3002").arg(inputManagerId)); // 中文: 未找到ID/用户名为 [%1] 的在职经理账号。
            return;
        }
    } else {
        const QString selectedManagerId = managerIds.value(managerCombo->currentIndex());
        for (const auto& user : m_repo->getAllUsers()) {
            if (user.getUserId() == selectedManagerId &&
                user.getRole() == UserRole::Manager &&
                user.isActive()) {
                managerFound = true;
                targetManager = user;
                break;
            }
        }
    }

    if (!managerFound) {
        QMessageBox::warning(this,
                             QStringLiteral("\u7ecf\u7406\u4e0d\u5b58\u5728"), // 中文: 经理不存在
                             QStringLiteral("\u9009\u4e2d\u7684\u7ecf\u7406\u8d26\u53f7\u4e0d\u5b58\u5728\u6216\u4e0d\u662f\u5728\u804c\u7ecf\u7406\u3002")); // 中文: 选中的经理账号不存在或不是在职经理。
        return;
    }

    targetManager.setDepartment(trimmedDepartment);
    if (!m_repo->saveUser(targetManager)) {
        QMessageBox::critical(this,
                              QStringLiteral("\u8bbe\u7f6e\u5931\u8d25"), // 中文: 设置失败
                              QStringLiteral("\u7ecf\u7406\u6240\u5c5e\u90e8\u95e8\u66f4\u65b0\u5931\u8d25\u3002")); // 中文: 经理所属部门更新失败。
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u8bbe\u7f6e\u6210\u529f"), // 中文: 设置成功
                             QStringLiteral("\u5df2\u5c06\u7ecf\u7406 [%1] \u8bbe\u4e3a\u90e8\u95e8 [%2] \u7684\u6240\u5c5e\u7ecf\u7406\u3002") // 中文: 已将经理 [%1] 设为部门 [%2] 的所属经理。
                                 .arg(targetManager.getUsername(), trimmedDepartment));
    renderDepartmentTree();
}

void AdminMainWindow::showAccountPasswordDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u7ba1\u7406\u5458/\u7ecf\u7406/\u9500\u552e\u8d26\u53f7\u5bc6\u7801")); // 中文: 管理员/经理/销售账号密码
    dialog.resize(760, 420);

    auto* layout = new QVBoxLayout(&dialog);
    auto* table = new QTableWidget(&dialog);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({
        QStringLiteral("\u7528\u6237ID"), // 中文: 用户ID
        QStringLiteral("\u7528\u6237\u540d"), // 中文: 用户名
        QStringLiteral("\u89d2\u8272"), // 中文: 角色
        QStringLiteral("\u90e8\u95e8"), // 中文: 部门
        QStringLiteral("\u72b6\u6001"), // 中文: 状态
        QStringLiteral("\u5bc6\u7801") // 中文: 密码
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table);

    int row = 0;
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() != UserRole::Admin &&
            user.getRole() != UserRole::Manager &&
            user.getRole() != UserRole::Sales) {
            continue;
        }

        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(user.getUserId()));
        table->setItem(row, 1, new QTableWidgetItem(user.getUsername()));
        QString roleText = QStringLiteral("\u9500\u552e"); // 中文: 销售
        if (user.getRole() == UserRole::Admin) {
            roleText = QStringLiteral("\u7ba1\u7406\u5458"); // 中文: 管理员
        } else if (user.getRole() == UserRole::Manager) {
            roleText = QStringLiteral("\u7ecf\u7406"); // 中文: 经理
        }
        table->setItem(row, 2, new QTableWidgetItem(roleText));
        table->setItem(row, 3, new QTableWidgetItem(user.getDepartment()));
        table->setItem(row, 4, new QTableWidgetItem(user.isActive()
                                                        ? QStringLiteral("\u5728\u804c") // 中文: 在职
                                                        : QStringLiteral("\u79bb\u804c"))); // 中文: 离职
        table->setItem(row, 5, new QTableWidgetItem(user.getPassword()));
        ++row;
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    dialog.exec();
}

void AdminMainWindow::generateRandomData()
{
    const QString message = QStringLiteral(
        "\u5c06\u6e05\u7a7a\u5f53\u524d\u5458\u5de5\u3001\u5ba2\u6237\u548c\u8ddf\u8fdb\u8bb0\u5f55\uff0c" // 中文: 将清空当前员工、客户和跟进记录，
        "\u5e76\u91cd\u65b0\u751f\u6210\uff1a\n\n" // 中文: 并重新生成：
        "\u7ba1\u7406\u5458 1 \u4e2a\n" // 中文: 管理员 1 个
        "\u7ecf\u7406 5 \u4e2a\n" // 中文: 经理 5 个
        "\u9500\u552e 20 \u4e2a\n" // 中文: 销售 20 个
        "\u5ba2\u6237 120 \u6761\n\n" // 中文: 客户 120 条
        "\u6240\u6709\u8d26\u53f7\u521d\u59cb\u5bc6\u7801\u90fd\u662f 123\u3002\u786e\u5b9a\u7ee7\u7eed\u5417\uff1f" // 中文: 所有账号初始密码都是 123。确定继续吗？
    );

    if (QMessageBox::warning(this,
                             QStringLiteral("\u751f\u6210\u6d4b\u8bd5\u6570\u636e"), // 中文: 生成测试数据
                             message,
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    RandomDataConfig config;
    config.adminCount = 1;
    config.managerCount = 5;
    config.salesCount = 20;
    config.customerCount = 120;

    if (!m_repo->seedRandomData(config, true)) {
        QMessageBox::critical(this,
                              QStringLiteral("\u751f\u6210\u5931\u8d25"), // 中文: 生成失败
                              QStringLiteral("\u6d4b\u8bd5\u6570\u636e\u5199\u5165\u672c\u5730\u6570\u636e\u5e93\u5931\u8d25\u3002")); // 中文: 测试数据写入本地数据库失败。
        return;
    }

    QMessageBox::information(this,
                             QStringLiteral("\u751f\u6210\u5b8c\u6210"), // 中文: 生成完成
                             QStringLiteral("\u5df2\u751f\u6210 1 \u4e2a\u7ba1\u7406\u5458\u30015 \u4e2a\u7ecf\u7406\u300120 \u4e2a\u9500\u552e\u548c 120 \u6761\u5ba2\u6237\u3002\n" // 中文: 已生成 1 个管理员、5 个经理、20 个销售和 120 条客户。
                                            "\u5efa\u8bae\u9000\u51fa\u5e76\u91cd\u65b0\u767b\u5f55\u4ee5\u4f7f\u5f53\u524d\u7528\u6237\u4e0a\u4e0b\u6587\u5b8c\u5168\u5237\u65b0\u3002")); // 中文: 建议退出并重新登录以使当前用户上下文完全刷新。

    refreshDataByMenu(m_leftMenu->currentRow());
}

void AdminMainWindow::showAddCustomerDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u6dfb\u52a0\u5ba2\u6237")); // 中文: 添加客户
    dialog.resize(420, 280);

    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* idEdit = new QLineEdit(&dialog);
    auto* nameEdit = new QLineEdit(&dialog);
    auto* phoneEdit = new QLineEdit(&dialog);
    auto* levelCombo = new QComboBox(&dialog);
    auto* ownerCombo = new QComboBox(&dialog);

    idEdit->setText(QStringLiteral("C%1").arg(QDateTime::currentMSecsSinceEpoch()));
    phoneEdit->setInputMask(QStringLiteral("00000000000"));
    phoneEdit->setPlaceholderText(QStringLiteral("请输入 11 位手机号"));
    phoneEdit->setText(QStringLiteral("13%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000000, 9, 10, QLatin1Char('0')));
    levelCombo->addItem(QStringLiteral("\u666e\u901a")); // 中文: 普通
    levelCombo->addItem(QStringLiteral("VIP"));
    ownerCombo->addItem(QStringLiteral("\u516c\u6d77\u6c60"), QString()); // 中文: 公海池
    for (const auto& user : m_repo->getAllUsers()) {
        if (user.getRole() == UserRole::Sales && user.isActive()) {
            ownerCombo->addItem(user.getUsername(), user.getUserId());
        }
    }

    form->addRow(QStringLiteral("\u5ba2\u6237\u7f16\u53f7:"), idEdit); // 中文: 客户编号:
    form->addRow(QStringLiteral("\u5ba2\u6237\u59d3\u540d:"), nameEdit); // 中文: 客户姓名:
    form->addRow(QStringLiteral("\u8054\u7cfb\u7535\u8bdd:"), phoneEdit); // 中文: 联系电话:
    form->addRow(QStringLiteral("\u5ba2\u6237\u7b49\u7ea7:"), levelCombo); // 中文: 客户等级:
    form->addRow(QStringLiteral("\u6240\u5c5e\u9500\u552e:"), ownerCombo); // 中文: 所属销售:
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        Customer customer(
            idEdit->text().trimmed(),
            nameEdit->text().trimmed(),
            phoneEdit->text().trimmed(),
            levelCombo->currentText(),
            QDateTime::currentDateTime(),
            ownerCombo->currentData().toString()
        );

        if (customer.getId().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog,
                                 QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                 QStringLiteral("\u5ba2\u6237\u7f16\u53f7\u4e0d\u80fd\u4e3a\u7a7a\u3002")); // 中文: 客户编号不能为空。
            return;
        }

        if (!customer.isValid()) {
            QMessageBox::warning(&dialog,
                                 QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                 QStringLiteral("\u5ba2\u6237\u59d3\u540d\u4e0d\u80fd\u4e3a\u7a7a\uff0c\u624b\u673a\u53f7\u5fc5\u987b\u662f 1 \u5f00\u5934\u7684 11 \u4f4d\u6570\u5b57\u3002")); // 中文: 客户姓名不能为空，手机号必须是 1 开头的 11 位数字。
            return;
        }

        for (const auto& existing : m_repo->getAllCustomers()) {
            if (existing.getId() == customer.getId()) {
                QMessageBox::warning(&dialog,
                                     QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                     QStringLiteral("\u5ba2\u6237\u7f16\u53f7\u5df2\u5b58\u5728\u3002")); // 中文: 客户编号已存在。
                return;
            }
        }

        if (!m_repo->saveCustomer(customer)) {
            QMessageBox::critical(&dialog,
                                  QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                  QStringLiteral("\u4fdd\u5b58\u5ba2\u6237\u5931\u8d25\uff0c\u8bf7\u68c0\u67e5\u672c\u5730\u6570\u636e\u5e93\u662f\u5426\u53ef\u5199\u3002")); // 中文: 保存客户失败，请检查本地数据库是否可写。
            return;
        }

        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QMessageBox::information(this, QStringLiteral("\u6dfb\u52a0\u6210\u529f"), // 中文: 添加成功
                             QStringLiteral("\u5ba2\u6237\u5df2\u6dfb\u52a0\u3002")); // 中文: 客户已添加。
    refreshDataByMenu(m_leftMenu->currentRow());
}

void AdminMainWindow::showAddSalesDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u6dfb\u52a0\u9500\u552e\u8d26\u53f7")); // 中文: 添加销售账号
    dialog.resize(420, 300);

    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* idEdit = new QLineEdit(&dialog);
    auto* nameEdit = new QLineEdit(&dialog);
    auto* deptEdit = new QLineEdit(&dialog);
    auto* passwordEdit = new QLineEdit(&dialog);

    idEdit->setText(QStringLiteral("sales%1").arg(QDateTime::currentSecsSinceEpoch()));
    deptEdit->setText(QStringLiteral("\u9500\u552e\u90e8")); // 中文: 销售部
    passwordEdit->setText(QStringLiteral("123"));
    passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow(QStringLiteral("\u9500\u552eID:"), idEdit); // 中文: 销售ID:
    form->addRow(QStringLiteral("\u767b\u5f55\u7528\u6237\u540d:"), nameEdit); // 中文: 登录用户名:
    form->addRow(QStringLiteral("\u6240\u5c5e\u90e8\u95e8:"), deptEdit); // 中文: 所属部门:
    form->addRow(QStringLiteral("\u767b\u5f55\u5bc6\u7801:"), passwordEdit); // 中文: 登录密码:
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    User user(idEdit->text().trimmed(),
              nameEdit->text().trimmed(),
              deptEdit->text().trimmed(),
              UserRole::Sales);
    user.setActive(true);

    if (user.getUserId().isEmpty() || user.getUsername().isEmpty() || passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                             QStringLiteral("\u9500\u552eID\u3001\u7528\u6237\u540d\u548c\u5bc6\u7801\u4e0d\u80fd\u4e3a\u7a7a\u3002")); // 中文: 销售ID、用户名和密码不能为空。
        return;
    }

    for (const auto& existing : m_repo->getAllUsers()) {
        if (existing.getUserId() == user.getUserId() || existing.getUsername() == user.getUsername()) {
            QMessageBox::warning(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                 QStringLiteral("\u9500\u552eID\u6216\u7528\u6237\u540d\u5df2\u5b58\u5728\u3002")); // 中文: 销售ID或用户名已存在。
            return;
        }
    }

    if (!m_repo->saveUser(user, passwordEdit->text())) {
        QMessageBox::critical(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                              QStringLiteral("\u4fdd\u5b58\u9500\u552e\u8d26\u53f7\u5931\u8d25\u3002")); // 中文: 保存销售账号失败。
        return;
    }

    QMessageBox::information(this, QStringLiteral("\u6dfb\u52a0\u6210\u529f"), // 中文: 添加成功
                             QStringLiteral("\u9500\u552e\u8d26\u53f7\u5df2\u6dfb\u52a0\u3002")); // 中文: 销售账号已添加。
    if (m_leftMenu->currentRow() == 1) {
        renderDepartmentTree();
    }
}

void AdminMainWindow::showAddManagerDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("\u6dfb\u52a0\u7ecf\u7406\u8d26\u53f7")); // 中文: 添加经理账号
    dialog.resize(420, 300);

    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    auto* idEdit = new QLineEdit(&dialog);
    auto* nameEdit = new QLineEdit(&dialog);
    auto* deptEdit = new QLineEdit(&dialog);
    auto* passwordEdit = new QLineEdit(&dialog);

    idEdit->setText(QStringLiteral("mgr_%1").arg(QDateTime::currentSecsSinceEpoch()));
    deptEdit->setText(QStringLiteral("\u9500\u552e\u90e8")); // 中文: 销售部
    passwordEdit->setText(QStringLiteral("123"));
    passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow(QStringLiteral("\u7ecf\u7406ID:"), idEdit); // 中文: 经理ID:
    form->addRow(QStringLiteral("\u767b\u5f55\u7528\u6237\u540d:"), nameEdit); // 中文: 登录用户名:
    form->addRow(QStringLiteral("\u6240\u5c5e\u90e8\u95e8:"), deptEdit); // 中文: 所属部门:
    form->addRow(QStringLiteral("\u767b\u5f55\u5bc6\u7801:"), passwordEdit); // 中文: 登录密码:
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    User user(idEdit->text().trimmed(),
              nameEdit->text().trimmed(),
              deptEdit->text().trimmed(),
              UserRole::Manager);
    user.setActive(true);

    if (user.getUserId().isEmpty() || user.getUsername().isEmpty() || passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                             QStringLiteral("\u7ecf\u7406ID\u3001\u7528\u6237\u540d\u548c\u5bc6\u7801\u4e0d\u80fd\u4e3a\u7a7a\u3002")); // 中文: 经理ID、用户名和密码不能为空。
        return;
    }

    for (const auto& existing : m_repo->getAllUsers()) {
        if (existing.getUserId() == user.getUserId() || existing.getUsername() == user.getUsername()) {
            QMessageBox::warning(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                                 QStringLiteral("\u7ecf\u7406ID\u6216\u7528\u6237\u540d\u5df2\u5b58\u5728\u3002")); // 中文: 经理ID或用户名已存在。
            return;
        }
    }

    if (!m_repo->saveUser(user, passwordEdit->text())) {
        QMessageBox::critical(this, QStringLiteral("\u6dfb\u52a0\u5931\u8d25"), // 中文: 添加失败
                              QStringLiteral("\u4fdd\u5b58\u7ecf\u7406\u8d26\u53f7\u5931\u8d25\u3002")); // 中文: 保存经理账号失败。
        return;
    }

    QMessageBox::information(this, QStringLiteral("\u6dfb\u52a0\u6210\u529f"), // 中文: 添加成功
                             QStringLiteral("\u7ecf\u7406\u8d26\u53f7\u5df2\u6dfb\u52a0\u3002")); // 中文: 经理账号已添加。
    if (m_leftMenu->currentRow() == 1) {
        renderDepartmentTree();
    }
}
