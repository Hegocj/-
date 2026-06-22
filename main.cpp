#include <QApplication>
#include <QMessageBox>
#include <memory>

// 1. 引入实体模型、数据接口与仓库实现
#include "GlobalModels.h"
#include "ICustomerRepository.h"
#include "SQLiteCustomerRepo.h"
#include "HighSeasManager.h"

// 2. 引入三个角色的隔离子类主窗口
#include "SalesMainWindow.h"
#include "ManagerMainWindow.h"
#include "adminmainwindow.h"

#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 【架构升级】：改用 std::shared_ptr 托管生命周期
    // 使用 SQLite 数据库，而不是 Mock 实现
    std::shared_ptr<ICustomerRepository> repo = std::make_shared<SQLiteCustomerRepo>();

    // 1. 初始化数据库（Mock 返回 true，未来切换到真实 SQLite 时无缝替换）
    if (!repo->initializeDatabase("crm.db")) {
        QMessageBox::critical(nullptr, "系统错误", "本地数据库初始化失败，程序退出！");
        return -1;
    }

    // 2. 弹出登录界面，传入 repo 指针供内部执行 checkLogin
    // 注意：如果你的 LoginDialog 构造函数收的是裸指针，可以用 repo.get() 传进去
    LoginDialog loginDlg(repo.get());

    // exec() 会阻塞，直到用户点击“登录”(Accepted) 或取消(Rejected)
    if (loginDlg.exec() == QDialog::Accepted) {

        // 3. 登录成功，获取回填的用户上下文（内含 Role 角色标签）
        User loggedInUser = loginDlg.getLoggedInUser();

        // 4. 【核心重构：多角色分流启动】
        // 利用多态，基类指针指向具体的角色子类实例
        BaseMainWindow* mainWin = nullptr;

        switch (loggedInUser.getRole()) {
        case UserRole::Sales:
            // 如果是销售，进入销售专属窗口
            mainWin = new SalesMainWindow(repo, loggedInUser);
            break;

        case UserRole::Manager:
            // 如果是经理，进入经理专属窗口（此处暂用 Sales 占位，等写完经理类直接替换）
            mainWin = new  ManagerMainWindow(repo, loggedInUser);
            break;

        case UserRole::Admin:
            // 如果是上帝管理员，进入全盘审计设置窗口
            mainWin = new AdminMainWindow(repo, loggedInUser);
            break;

        default:
            QMessageBox::warning(nullptr, "权限错误", "未知的用户角色，安全拦截！");
            repo->closeDatabase();
            return -1;
        }

        // 5. 显示对应权限的窗口，进入 Qt 主事件循环
        if (mainWin)
            //  关键一步：对象已经安全诞生，通知基类点火启动菜单和数据刷新
            mainWin->bootstrapMainWindow();
        auto* highSeasManager = new HighSeasManager(repo, 30, mainWin);
        highSeasManager->start(5 * 60 * 1000);
        mainWin->show();
        int result = a.exec();

        // 6. 优雅安全清理
        delete mainWin; // 析构主窗口
        repo->closeDatabase(); // 关闭数据库连接（shared_ptr 会在离开 main 后自动释放内存）

        return result;
    }

    // 用户取消登录，关闭数据库并安全退出
    repo->closeDatabase();
    return 0;
}
