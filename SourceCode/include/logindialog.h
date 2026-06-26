/**
 * @file logindialog.h
 * @brief 系统登录弹窗的类声明。
 *
 * 研究定位：本文件属于表示层中的身份认证入口，负责定义用户输入账号密码并获取
 * 当前用户上下文的界面对象。登录成功后的 User 会决定后续进入哪一种主窗口。
 *
 * 主要职责：保存仓库指针、登录成功用户对象，声明登录提交槽函数，并向 main.cpp
 * 提供 getLoggedInUser() 获取认证后的完整用户信息。
 */
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ICustomerRepository.h"
#include "GlobalModels.h"

namespace Ui { class LoginDialog; }

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(ICustomerRepository* repo, QWidget *parent = nullptr);
    ~LoginDialog();

    // 供 main.cpp 提取完整合法的用户信息（含 ID 和 隔离角色）
    User getLoggedInUser() const { return m_loggedInUser; }

private slots:
    // 【核心修正】：不再使用 on_loginButton_clicked 这种自动前缀，防止解绑 Bug
    void handleLoginSubmit();

private:
    Ui::LoginDialog *ui;
    ICustomerRepository* m_repo;
    User m_loggedInUser; // 完美拦截并保存成功回填的用户
};

#endif // LOGINDIALOG_H
