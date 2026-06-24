/**
 * @file ManagerMainWindow.h
 * @brief 经理主界面的类声明。
 *
 * 研究定位：本文件属于表示层中的经理权限模块，负责定义部门经理查看团队成员、
 * 管理部门客户和进行客户分配的工作台结构。
 *
 * 主要职责：声明经理菜单刷新、部门范围搜索、团队成员渲染、客户列表渲染和客户
 * 负载均衡分配方法。经理窗口继承 BaseMainWindow，并通过仓库接口限定数据范围。
 */
#ifndef MANAGERMAINWINDOW_H
#define MANAGERMAINWINDOW_H

#include "BaseMainWindow.h"
#include <map>

class ManagerMainWindow : public BaseMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 经理子类构造函数
     * @param repo 依赖注入的数据仓库
     * @param user 当前登录的经理实体
     */
    ManagerMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent = nullptr);
    ~ManagerMainWindow() override = default;

protected:
    // =========================================================================
    // 权限隔离：重写基类外包的 4 个核心纯虚函数
    // =========================================================================
    void initRoleMenu() override;                    // 注入经理专属三菜单
    void refreshDataByMenu(int index) override;       // 根据菜单索引切换不同的渲染策略
    void executeSearch(const QString& key) override;  // 部门限制的模糊搜索
    void executeRowModification(int row) override;    // 经理特有的双击流转与一键交接业务
    
private:
    // =========================================================================
    //️ 经理特有私有辅助渲染与算法函数
    // =========================================================================
    void renderTeamUsers();                           // 渲染下属团队员工列表与负荷
    void renderTeamUsers(const std ::vector<User>& indicatedUsers); //重载renderTeamUsers,显示出搜索结果
    void renderCustomers(const std::vector<Customer>& customers); // 渲染客户大表

    /**
     * @brief 核心算法：弹出指派窗口并实时动态计算、展示在职销售的当前跟进负荷
     * @param customerId 准备被指派的客户ID
     * @return 是否成功完成指派变更
     */
    bool allocateCustomerWithLoadBalancing(const QString& customerId);
    
private:
    // 缓存数据，防止行号错位
    std::vector<User> m_teamUsers;
    std::vector<Customer> m_displayedCustomers;
};

#endif // MANAGERMAINWINDOW_H
