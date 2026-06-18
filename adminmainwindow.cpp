#include "AdminMainWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

// 管理员私有变量声明（可以在类的构造函数中动态初始化）
static QTreeWidget* s_orgTree = nullptr;

AdminMainWindow::AdminMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : BaseMainWindow(repo, user, parent)
{


    // 初始化树状控件，并塞入原本放置 Table 的中心容器中
    if (m_customerTable && m_customerTable->parentWidget()) {
        s_orgTree = new QTreeWidget(m_customerTable->parentWidget());
        s_orgTree->setColumnCount(3);
        s_orgTree->setHeaderLabels({"部门", "员工ID", "职权"});
        s_orgTree->hide(); // 默认先隐藏

        // 寻找布局并把树加进去（使其与 Table 共享空间）
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
        "全部客户",
        "全部部门", // 树状层级展开视图
        "公海池"
    });
}

// =========================================================================
//  2. 视图切换：在【常规大表】与【树状架构】之间做显隐无缝切换
// =========================================================================
void AdminMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear();

    // 安全控制：切换菜单时显隐互斥
    if (index == 1) {
        if (m_customerTable) m_customerTable->hide();
        if (s_orgTree) {
            s_orgTree->show();
            renderDepartmentTree(); // 触发树状构造
        }
    } else {
        if (s_orgTree) s_orgTree->hide();
        if (m_customerTable) {
            m_customerTable->show();
            m_globalCustomersCache = (index == 0) ? m_repo->getAllCustomers() : m_repo->getHighSeasCustomers();
            renderGlobalCustomers(m_globalCustomersCache);
        }
    }
}

// =========================================================================
// 3. 核心功能：构建带 `>` 箭头的组织架构树
// =========================================================================
void AdminMainWindow::renderDepartmentTree()
{
    if (!s_orgTree) return;
    s_orgTree->clear();

    std::vector<User> allUsers = m_repo->getAllUsers();

    // 1. 提取所有经理（作为父节点）
    for (const auto& user : allUsers) {
        if (user.getRole() == UserRole::Manager) {

            // 创建顶级父节点（部门/经理节点），Qt 会自动在前面挂载 `>` 箭头
            QTreeWidgetItem* managerItem = new QTreeWidgetItem(s_orgTree);
            managerItem->setText(0, user.getDepartment() + " [Manager: " + user.getUsername() + "]");
            managerItem->setText(1, user.getUserId());
            managerItem->setText(2, "Active Manager");

            // 2. 嵌套提取该经理部门底下的所有销售员工（作为子节点）
            for (const auto& subUser : allUsers) {
                if (subUser.getDepartment() == user.getDepartment() && subUser.getRole() == UserRole::Sales) {

                    // 将其挂载到刚才的 managerItem 下面，作为子节点隐藏起来
                    QTreeWidgetItem* salesItem = new QTreeWidgetItem(managerItem);
                    salesItem->setText(0, "  Sales Staff: " + subUser.getUsername());
                    salesItem->setText(1, subUser.getUserId());
                    salesItem->setText(2, subUser.isActive() ? "Active" : "Disabled");
                }
            }
        }
    }

    // 默认收起所有节点，留出 `>` 供管理员自主点击点击展开
    s_orgTree->collapseAll();
}

// =========================================================================
//  4. 搜索适配
// =========================================================================
void AdminMainWindow::executeSearch(const QString& key)
{
    if (key.isEmpty()) {
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    int currentMenuIndex = m_leftMenu->currentRow();

    if (currentMenuIndex == 1) {
        // 在树状图里模糊匹配对应的节点，匹配不上的隐藏
        if (!s_orgTree) return;
        for (int i = 0; i < s_orgTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* parent = s_orgTree->topLevelItem(i);
            bool parentMatch = parent->text(0).contains(key) || parent->text(1).contains(key);

            int visibleChildren = 0;
            for (int j = 0; j < parent->childCount(); ++j) {
                QTreeWidgetItem* child = parent->child(j);
                bool childMatch = child->text(0).contains(key) || child->text(1).contains(key);
                child->setHidden(!childMatch);
                if (childMatch) visibleChildren++;
            }

            // 如果父节点中招或者子节点有中招的，就展开并显示此树枝
            if (parentMatch || visibleChildren > 0) {
                parent->setHidden(false);
                parent->setExpanded(true);
            } else {
                parent->setHidden(true);
            }
        }
    }
    else {
        // 常规客户大盘的内存过滤
        std::vector<Customer> source = (currentMenuIndex == 0) ? m_repo->getAllCustomers() : m_repo->getHighSeasCustomers();
        std::vector<Customer> filtered;
        for (const auto& c : source) {
            if (c.getName().contains(key) || c.getId().contains(key)) {
                filtered.push_back(c);
            }
        }
        renderGlobalCustomers(filtered);
    }
}

// =========================================================================
//  5. 双击事件：双击不承担展开工作（展开交给>），双击专心弹窗修改配置
// =========================================================================
void AdminMainWindow::executeRowModification(int row)
{
    // 当管理员看的是全盘客户或系统公海表格时
    if (m_leftMenu->currentRow() == 0 || m_leftMenu->currentRow() == 2) {
        if (row < 0 || row >= static_cast<int>(m_globalCustomersCache.size())) return;

        CustomerDetailDialog detailDlg(m_globalCustomersCache[row].getId(), m_repo, m_currentUser, this);
        if (detailDlg.exec() == QDialog::Accepted) {
            refreshDataByMenu(m_leftMenu->currentRow());
        }
    }
}

void AdminMainWindow::renderGlobalCustomers(const std::vector<Customer>& customers)
{
    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({"Customer ID", "Customer Name", "Contact Phone", "Current Owner ID"});

    for (size_t i = 0; i < customers.size(); ++i) {
        m_customerTable->insertRow(i);
        m_customerTable->setItem(i, 0, new QTableWidgetItem(customers[i].getId()));
        m_customerTable->setItem(i, 1, new QTableWidgetItem(customers[i].getName()));
        m_customerTable->setItem(i, 2, new QTableWidgetItem(customers[i].getPhone()));
        m_customerTable->setItem(i, 3, new QTableWidgetItem(customers[i].getOwnerId().isEmpty() ? "Unassigned" : customers[i].getOwnerId()));
    }
}