/**
 * @file userdetaildialog.h
 * @brief 员工档案与账号维护弹窗的类声明。
 *
 * 研究定位：本文件属于表示层中的员工管理模块，负责定义管理员或经理查看、修改、
 * 离职交接和删除员工档案的弹窗结构。
 *
 * 主要职责：声明员工资料加载、保存修改、离职并转移客户、删除员工等操作。它通过
 * ICustomerRepository 读写用户信息，并与客户资产交接接口协作。
 */
#ifndef USERDETAILDIALOG_H
#define USERDETAILDIALOG_H

#include "GlobalModels.h"
#include "ICustomerRepository.h"

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <memory>

class UserDetailDialog : public QDialog
{
    Q_OBJECT

public:
    UserDetailDialog(const QString& targetUserId,
                     std::shared_ptr<ICustomerRepository> repo,
                     const User& currentUser,
                     QWidget* parent = nullptr);

private slots:
    void onSaveButtonClicked();
    void onTransferButtonClicked();
    void onDeleteButtonClicked();

private:
    void setupUi();
    void loadUserData();

    QString m_targetUserId;
    std::shared_ptr<ICustomerRepository> m_repo;
    User m_currentUser;
    User m_targetUser;

    QLineEdit* m_idEdit = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_deptEdit = nullptr;
    QComboBox* m_roleCombo = nullptr;
    QLineEdit* m_statusEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_transferBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
};

#endif // USERDETAILDIALOG_H
