/**
 * @file BaseMainWindow.h
 * @brief 三类角色主窗口的公共抽象基类声明。
 *
 * 研究定位：本文件属于表示层的主框架模块，承担销售、经理、管理员三个角色界面的
 * 公共骨架设计。它将窗口布局、搜索、防抖、导入导出和登出流程集中封装，角色差异
 * 通过纯虚函数交给子类实现。
 *
 * 主要职责：定义统一主窗口控件、仓库依赖、当前用户上下文，以及角色菜单初始化、
 * 菜单切换刷新、搜索执行和表格行操作四个扩展点。该类是界面多态结构的核心。
 */
#ifndef BASEMAINWINDOW_H
#define BASEMAINWINDOW_H

#include "ICustomerRepository.h"
#include "GlobalModels.h"
#include "customerdetaildialog.h"
#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <memory>


namespace Ui {
class MainWindow;
}

class BaseMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param repo 依赖注入的数据仓库指针（支持 Mock 或真实 SQLite）
     * @param user 当前登录成功的用户实体上下文
     */
    BaseMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent = nullptr);
    virtual ~BaseMainWindow();

    void bootstrapMainWindow();

signals:
    void logoutRequested();

protected:
    // =========================================================================
    //  Sandbox 空间套娃架构核心隔离逻辑
    // =========================================================================
    /**
     * @brief 借鉴精美布局逻辑，无损反向注入到 mainPlaceholder 中
     */
    void initMixLayout();

    /**
     * @brief 统一拦截顶层原生信号，并绑定到基类虚空调度槽函数上
     */
    void bindCoreSignals();

    // =========================================================================
    //  权限隔离：留给 3 个角色子类填空的 4 个核心多态纯虚函数
    // =========================================================================
    virtual void initRoleMenu() = 0;                    // 子类决定自己左侧菜单长什么样
    virtual void refreshDataByMenu(int index) = 0;       // 子类根据左侧菜单点击切换右侧数据
    virtual void executeSearch(const QString& key) = 0;  // 子类根据自身权限执行带隔离的搜索
    virtual void executeRowModification(int row) = 0;    // 子类实现行双击后的特权或常规流转业务

    // =========================================================================
    //  基类通用特权工具箱（对子类开放，后续业务弹窗直接调用）
    // =========================================================================
    /**
     * @brief 弹出全景跟进记录时间轴弹窗
     * @param enableAssignButton 是否亮起特权分配按钮（Sales传false，Manager/Admin传true）
     */
    bool showFollowTimelineDialog(const QString& customerId, bool enableAssignButton);

protected slots:
    // =========================================================================
    //  统一防抖调度与状态回流机制
    // =========================================================================
    void onSearchTextChanged(const QString& text); // 输入框打字拦截
    void onDebounceTimerTimeout();                  // 防抖时间到，触发子类具体搜索
    void onMenuIndexChanged(int index);             // 侧边栏切换拦截，触发子类具体刷新
    void onTableDoubleClicked(int row, int column); // 行双击拦截，触发子类具体流转

    //virtual void onGlobalImportClicked() = 0; // 声明为纯虚或虚函数，让经理/管理员子类去重写实现
    //virtual void onGlobalExportClicked() = 0;

protected:
    // 数据隔离上下文
    std::shared_ptr<ICustomerRepository> m_repo;
    User m_currentUser;

    // 完美保留的精美 UI 控件指针（受保护的权限，子类可随时读取/操作）
    QSplitter* m_splitter = nullptr;
    QListWidget* m_leftMenu = nullptr;
    QWidget* m_rightContainer = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QPushButton* m_searchButton = nullptr;
    QTableWidget* m_customerTable = nullptr;
    QPushButton* m_importBtn = nullptr;
    QPushButton* m_exportBtn = nullptr;
    QPushButton* m_logoutBtn = nullptr;

private:
    Ui::MainWindow *ui;
    QTimer* m_debounceTimer = nullptr; // 防抖定时器
    QString m_pendingSearchKey;         // 缓存的防抖输入字串
};

#endif // BASEMAINWINDOW_H
