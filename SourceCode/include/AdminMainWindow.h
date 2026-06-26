/**
 * @file AdminMainWindow.h
 * @brief 管理员主界面的类声明。
 *
 * 研究定位：本文件属于表示层中的管理员权限模块，负责定义系统管理员能够使用的客户、
 * 部门、账号维护入口。它继承 BaseMainWindow，在统一主窗口框架上扩展全局数据管理能力。
 *
 * 主要职责：声明管理员菜单初始化、全局客户列表渲染、组织结构树维护、账号密码查看、
 * 新增客户与新增员工等接口。该类本身不直接操作数据库，而是通过 ICustomerRepository
 * 完成数据读写，以保持界面逻辑和数据访问逻辑分离。
 */
#ifndef ADMINMAINWINDOW_H
#define ADMINMAINWINDOW_H

#include "BaseMainWindow.h"

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class AdminMainWindow : public BaseMainWindow
{
    Q_OBJECT

public:
    AdminMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget* parent = nullptr);
    ~AdminMainWindow() override = default;

protected:
    void initRoleMenu() override;
    void refreshDataByMenu(int index) override;
    void executeSearch(const QString& key) override;
    void executeRowModification(int row) override;

private slots:
    void onTreeItemDoubleClicked(QTreeWidgetItem* item, int column);
    void showAccountPasswordDialog();
    void showAddCustomerDialog();
    void showAddSalesDialog();
    void showAddManagerDialog();
    void generateRandomData();

private:
    void renderDepartmentTree();
    void renderGlobalCustomers(const std::vector<Customer>& customers);
    void showDepartmentManagerDialog(const QString& department);

    std::vector<Customer> m_globalCustomersCache;
    QPushButton* m_accountPasswordBtn = nullptr;
    QPushButton* m_addCustomerBtn = nullptr;
    QPushButton* m_addSalesBtn = nullptr;
    QPushButton* m_addManagerBtn = nullptr;
    QPushButton* m_generateDataBtn = nullptr;
};

#endif // ADMINMAINWINDOW_H
