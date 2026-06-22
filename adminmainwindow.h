#ifndef ADMINMAINWINDOW_H
#define ADMINMAINWINDOW_H

#include "BaseMainWindow.h"

// 前置声明 Qt 树状控件，避免在头文件中包含过重的 UI 库
class QTreeWidget;
class QTreeWidgetItem;

class AdminMainWindow : public BaseMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 管理员子类构造函数
     * @param repo 依赖注入的数据仓库
     * @param user 当前登录的管理员实体
     */
    AdminMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent = nullptr);
    ~AdminMainWindow() override = default;

protected:
    // =========================================================================
    // 权限隔离：重写基类外包的 4 个核心纯虚函数
    // =========================================================================
    void initRoleMenu() override;                    // 注入管理员专属三菜单
    void refreshDataByMenu(int index) override;       // 根据选择维度切换大表或树状图
    void executeSearch(const QString& key) override;  // 内存级全局模糊搜索（支持树节点过滤）
    void executeRowModification(int row) override;    // 双击具体的客户行或选中的销售树节点
    
private slots:
    void onTreeItemDoubleClicked(QTreeWidgetItem* item, int column); // 树状视图双击事件

private:
    // =========================================================================
    // 管理员独有的层级拓扑树渲染函数
    // =========================================================================
    void renderDepartmentTree();                      // 构建带 > 箭头的 经理->销售 两级树状组织
    void renderGlobalCustomers(const std::vector<Customer>& customers); // 渲染标准客户表
    

private:
    // 客户大盘数据缓存，用于绝对安全的行号映射
    std::vector<Customer> m_globalCustomersCache;
};

#endif // ADMINMAINWINDOW_H