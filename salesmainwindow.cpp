#include "SalesMainWindow.h"
#include <QMessageBox>

SalesMainWindow::SalesMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : BaseMainWindow(repo, user, parent)
{

    // 2. 默认选中第一个菜单（我的私海客户），触发初始数据加载
    if (m_leftMenu->count() > 0) {
        m_leftMenu->setCurrentRow(0);
    }
}

// =========================================================================
//  1. 菜单注入：划定销售的业务可见边界
// =========================================================================
void SalesMainWindow::initRoleMenu()
{
    m_leftMenu->clear();
    m_leftMenu->addItems({
        " 我的客户",
        "系统公海池"
    });
}

// =========================================================================
//  2. 菜单切换：无缝读取私海或公海
// =========================================================================
void SalesMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear(); // 切换菜单时清空搜索框

    if (index == 0) {
        // 我的客户：从 repo 中仅读取 owner_id == 当前销售工号的数据
        m_displayedCustomers = m_repo->getCustomersBySales(m_currentUser.getUserId());
    }
    else if (index == 1) {
        // 系统公海池：从 repo 中读取 owner_id 为空的数据
        m_displayedCustomers = m_repo->getHighSeasCustomers();
    }

    // 将数据渲染到大表上
    renderTableData(m_displayedCustomers);
}

// =========================================================================
//  3. 权限隔离搜索：调用 Repo 并传递当前 User，防止越权
// =========================================================================
void SalesMainWindow::executeSearch(const QString& key)
{
    if (key.isEmpty()) {
        // 如果清空了搜索框，自动还原当前菜单的视图
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    // 后端 searchCustomers 会根据 m_currentUser 的 Role 自动拼接：
    // WHERE (owner_id = '自己' OR owner_id = '') AND (name LIKE %key% OR phone LIKE %key%)
    m_displayedCustomers = m_repo->searchCustomers(key, m_currentUser);

    renderTableData(m_displayedCustomers);
}

// =========================================================================
//  4. 行双击特权流转：销售的核心对客流转逻辑（全新升级版）
// =========================================================================
void SalesMainWindow::executeRowModification(int row)
{
    if (row < 0 || row >= static_cast<int>(m_displayedCustomers.size())) return;

    // 不管是私海还是公海，直接呼叫统一详情弹窗
    CustomerDetailDialog detailDlg(m_displayedCustomers[row].getId(), m_repo, m_currentUser, this);

    if (detailDlg.exec() == QDialog::Accepted) {
        // 退出后一律自动根据当前菜单重刷主表，极为优雅
        refreshDataByMenu(m_leftMenu->currentRow());
    }
}

// =========================================================================
//️ 辅助渲染工具：将底层实体洗入 QTableWidget
// =========================================================================
void SalesMainWindow::renderTableData(const std::vector<Customer>& customers)
{
    m_customerTable->setRowCount(0); // 清空旧行

    for (size_t i = 0; i < customers.size(); ++i) {
        m_customerTable->insertRow(i);

        // 1. 客户ID
        m_customerTable->setItem(i, 0, new QTableWidgetItem(customers[i].getId()));
        //debug
        qDebug()<<" 客户ID";

        // 2. 客户姓名
        m_customerTable->setItem(i, 1, new QTableWidgetItem(customers[i].getName()));
        //debug
         qDebug()<<" 客户姓名";
        // 3. 联系电话
        m_customerTable->setItem(i, 2, new QTableWidgetItem(customers[i].getPhone()));
        //debug
         qDebug()<<" 客户电话";

        // 4. 跟进状态 / 级别（若是公海客户，状态栏特殊高亮显示醒目提示）
        QTableWidgetItem* levelItem = new QTableWidgetItem(customers[i].getLevel());
        if (customers[i].getOwnerId().isEmpty()) {
            levelItem->setText("[公海池] " + customers[i].getLevel());
            levelItem->setForeground(QBrush(Qt::yellow)); // 公海客户用黄色字凸显
        }
        m_customerTable->setItem(i, 3, levelItem);
    }
}