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
