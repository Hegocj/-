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