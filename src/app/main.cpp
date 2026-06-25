/**
 * @file main.cpp
 * @brief 应用程序入口与主流程调度。
 *
 * 研究定位：本文件是客户信息管理系统的启动入口，负责组织数据库初始化、用户登录、
 * 角色主窗口选择和公海池后台任务启动。它把系统各层模块串联成可运行程序。
 *
 * 主要职责：创建 QApplication，初始化 SQLiteCustomerRepo，循环处理登录/登出，
 * 根据 UserRole 创建 SalesMainWindow、ManagerMainWindow 或 AdminMainWindow，
 * 并为每个主窗口启动 HighSeasManager 定时回收机制。
 *
 * 编码说明：本文件中形如 "\u4e2d\u6587" 的内容是 Unicode 转义字符串，
 * 编译运行后会显示为中文，用来避免不同编辑器编码设置导致界面文字乱码。
 */
#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include <memory>

#include "GlobalModels.h"
#include "HighSeasManager.h"
#include "ICustomerRepository.h"
#include "SQLiteCustomerRepo.h"

#include "ManagerMainWindow.h"
#include "SalesMainWindow.h"
#include "AdminMainWindow.h"
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    RandomDataConfig randomDataConfig;
    randomDataConfig.customerCount = 120;
    randomDataConfig.salesCount = 20;
    randomDataConfig.managerCount = 5;
    randomDataConfig.adminCount = 1;

    std::shared_ptr<ICustomerRepository> repo = std::make_shared<SQLiteCustomerRepo>(randomDataConfig);
    if (!repo->initializeDatabase(QStringLiteral("crm.db"))) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("\u7cfb\u7edf\u9519\u8bef"), // 中文: 系统错误
                              QStringLiteral("\u672c\u5730\u6570\u636e\u5e93\u521d\u59cb\u5316\u5931\u8d25\uff0c\u7a0b\u5e8f\u9000\u51fa\uff01")); // 中文: 本地数据库初始化失败，程序退出！
        return -1;
    }

    int result = 0;
    bool relogin = true;

    while (relogin) {
        relogin = false;

        LoginDialog loginDlg(repo.get());
        if (loginDlg.exec() != QDialog::Accepted) {
            break;
        }

        const User loggedInUser = loginDlg.getLoggedInUser();
        BaseMainWindow* mainWin = nullptr;

        switch (loggedInUser.getRole()) {
        case UserRole::Sales:
            mainWin = new SalesMainWindow(repo, loggedInUser);
            break;
        case UserRole::Manager:
            mainWin = new ManagerMainWindow(repo, loggedInUser);
            break;
        case UserRole::Admin:
            mainWin = new AdminMainWindow(repo, loggedInUser);
            break;
        default:
            QMessageBox::warning(nullptr,
                                 QStringLiteral("\u6743\u9650\u9519\u8bef"), // 中文: 权限错误
                                 QStringLiteral("\u672a\u77e5\u7684\u7528\u6237\u89d2\u8272\uff0c\u5df2\u62e6\u622a\u3002")); // 中文: 未知的用户角色，已拦截。
            repo->closeDatabase();
            return -1;
        }

        QObject::connect(mainWin, &BaseMainWindow::logoutRequested, [&relogin]() {
            relogin = true;
        });

        mainWin->bootstrapMainWindow();
        auto* highSeasManager = new HighSeasManager(repo, 30, mainWin);
        highSeasManager->start(5 * 60 * 1000);

        QEventLoop windowLoop;
        mainWin->setAttribute(Qt::WA_DeleteOnClose);
        QObject::connect(mainWin, &QObject::destroyed, &windowLoop, &QEventLoop::quit);

        mainWin->show();
        result = windowLoop.exec();
    }

    repo->closeDatabase();
    return result;
}
