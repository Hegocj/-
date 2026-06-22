#ifndef USERDETAILDIALOG_H
#define USERDETAILDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "GlobalModels.h"
#include "ICustomerRepository.h"   // 用于调用底层数据库或流转接口
#include <memory>

class UserDetailDialog : public QDialog {
    Q_OBJECT
public:
    // 构造函数：传入目标员工ID、数据仓库、以及当前登录用户（用于权限判定）
    UserDetailDialog(const QString& targetUserId, std::shared_ptr<ICustomerRepository> repo, const User& currentUser, QWidget* parent = nullptr);

private slots:
    void onTransferButtonClicked(); // 离职转移核心槽函数
    void onDeleteButtonClicked();

private:
    void setupUi();       // 纯代码绘制布局，规避 .ui 文件的依赖
    void loadUserData();  // 加载显示目标员工的信息

    QString m_targetUserId;
    std::shared_ptr<ICustomerRepository> m_repo;
    User m_currentUser;   // 当前操作人（判定是经理还是销售）
    User m_targetUser;    // 被查看的目标员工对象

    // UI 控件
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_deptEdit;
    QLineEdit* m_roleEdit;
    QLineEdit* m_statusEdit;
    QPushButton* m_transferBtn;
    QPushButton* m_closeBtn;
    QPushButton* m_deleteBtn;
};
#endif // USERDETAILDIALOG_H
