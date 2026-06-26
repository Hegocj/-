/**
 * @file customerdetaildialog.h
 * @brief 客户详情与客户流转弹窗的类声明。
 *
 * 研究定位：本文件属于表示层中的客户明细模块，负责定义单个客户的查看、编辑、
 * 认领、释放、删除和跟进记录入口。它根据当前登录用户角色动态控制可用操作。
 *
 * 主要职责：声明客户资料加载、权限控制、负责人下拉框填充、保存客户、公海认领、
 * 释放到公海以及打开跟进时间线等行为。数据读写通过 ICustomerRepository 完成。
 */
#ifndef CUSTOMERDETAILDIALOG_H
#define CUSTOMERDETAILDIALOG_H

#include <QDialog>
#include <memory>
#include <vector>
#include "ICustomerRepository.h"
#include "GlobalModels.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class CustomerDetailDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 统一全功能客户详情构造函数
     * @param customerId 目标客户ID
     * @param repo 仓库指针
     * @param currentUser 当前操作人上下文（决定其在弹窗内的特权级别）
     */
    CustomerDetailDialog(const QString& customerId,
                         std::shared_ptr<ICustomerRepository> repo,
                         const User& currentUser,
                         QWidget *parent = nullptr);
    ~CustomerDetailDialog() override = default;

private slots:
    void handleSaveOrCommit();         // 点击保存/流转主按钮
    void handleClaimAction();          // 公海特权：认领该客户
    void handleEvictAction();          // 经理特权：强行放逐到公海
    void openFollowTimeline();         // 打开时间轴弹窗

private:
    void loadCustomerAndSetupPrivilege(); // 核心：注入数据并实施动态权限置灰
    void populateSalesCombo();            // 经理特权：洗出旗下所有销售员工到下拉框

private:
    QString m_customerId;
    std::shared_ptr<ICustomerRepository> m_repo;
    User m_currentUser;
    Customer m_customer;

    // 统一大表单控件
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_phoneEdit;
    QComboBox* m_levelCombo;

    // 关键改变：负责人变成下拉框，经理可双击或点击直接修改销售
    QComboBox* m_ownerCombo;

    // 功能按钮底座
    QPushButton* m_followBtn;   // 跟进记录
    QPushButton* m_claimBtn;    // 认领按钮（公海专属）
    QPushButton* m_evictBtn;    // 踢回公海按钮（经理/管理员特权）
    QPushButton* m_saveBtn;     // 保存
    QPushButton* m_cancelBtn;   // 取消
    QPushButton* m_deleteBtn;   // 删除
};

#endif // CUSTOMERDETAILDIALOG_H
