/**
 * @file SalesMainWindow.h
 * @brief 销售主界面的类声明。
 *
 * 研究定位：本文件属于表示层中的销售权限模块，负责定义销售人员查看本人客户和
 * 系统公海池的工作台结构。
 *
 * 主要职责：声明销售菜单初始化、客户数据刷新、权限范围内搜索、客户行双击处理和
 * 客户表格渲染方法。销售窗口继承 BaseMainWindow，但只能操作自己负责或可认领的客户。
 */
#ifndef SALESMAINWINDOW_H
#define SALESMAINWINDOW_H

#include "BaseMainWindow.h"

class SalesMainWindow : public BaseMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 销售子类构造函数
     * @param repo 依赖注入的数据仓库指针
     * @param user 当前登录的销售员工实体
     */
    SalesMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent = nullptr);
    ~SalesMainWindow() override = default;

protected:
    // =========================================================================
    //  实现基类外包的 4 个核心纯虚函数（多态注入）
    // =========================================================================
    void initRoleMenu() override;                    // 注入销售专属双菜单
    void refreshDataByMenu(int index) override;       // 根据菜单索引切换私海或公海数据
    void executeSearch(const QString& key) override;  // 销售权限受限的模糊搜索
    void executeRowModification(int row) override;    // 销售特有的双击流转业务（写跟进/捞人）

private:
    /**
     * @brief 统一将 Customer 列表渲染到右侧 QTableWidget 中
     */
    void renderTableData(const std::vector<Customer>& customers);

private:
    // 缓存当前右侧大表里正在展示的客户实体，方便双击行时直接通过行号（Row）捞出数据
    std::vector<Customer> m_displayedCustomers;
};

#endif // SALESMAINWINDOW_H
