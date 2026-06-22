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

private:
    void renderDepartmentTree();
    void renderGlobalCustomers(const std::vector<Customer>& customers);
    void showDepartmentManagerDialog(const QString& department);

    std::vector<Customer> m_globalCustomersCache;
    QPushButton* m_accountPasswordBtn = nullptr;
    QPushButton* m_addCustomerBtn = nullptr;
    QPushButton* m_addSalesBtn = nullptr;
    QPushButton* m_addManagerBtn = nullptr;
};

#endif // ADMINMAINWINDOW_H
