#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(ICustomerRepository* repo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_repo(repo)
{
    ui->setupUi(this);

    resize(400, 300);
    setWindowTitle(QStringLiteral("\u7cfb\u7edf\u767b\u5f55"));

    QWidget* centerContainer = new QWidget(this);
    centerContainer->setFixedWidth(280);

    auto* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(15);

    auto* formLayout = new QGridLayout();
    formLayout->setSpacing(10);

    ui->label_username->setFixedWidth(55);
    ui->label_userpassword->setFixedWidth(55);
    ui->label_username->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_userpassword->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_username->setText(QStringLiteral("\u7528\u6237\u540d"));
    ui->label_userpassword->setText(QStringLiteral("\u5bc6\u7801"));
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    ui->label_username->setParent(centerContainer);
    ui->usernameEdit->setParent(centerContainer);
    ui->label_userpassword->setParent(centerContainer);
    ui->passwordEdit->setParent(centerContainer);
    ui->loginButton->setParent(centerContainer);

    formLayout->addWidget(ui->label_username, 0, 0);
    formLayout->addWidget(ui->usernameEdit, 0, 1);
    formLayout->addWidget(ui->label_userpassword, 1, 0);
    formLayout->addWidget(ui->passwordEdit, 1, 1);

    centerLayout->addLayout(formLayout);

    ui->loginButton->setFixedHeight(35);
    ui->loginButton->setText(QStringLiteral("\u767b\u5f55"));
    centerLayout->addWidget(ui->loginButton);

    auto* globalHorizontalLayout = new QHBoxLayout();
    globalHorizontalLayout->addStretch(1);
    globalHorizontalLayout->addWidget(centerContainer);
    globalHorizontalLayout->addStretch(1);

    if (layout()) {
        delete layout();
    }

    auto* globalMainLayout = new QVBoxLayout(this);
    globalMainLayout->addStretch(1);
    globalMainLayout->addLayout(globalHorizontalLayout);
    globalMainLayout->addStretch(1);

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::handleLoginSubmit);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::handleLoginSubmit()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this,
                             QStringLiteral("\u63d0\u793a"),
                             QStringLiteral("\u7528\u6237\u540d\u6216\u5bc6\u7801\u4e0d\u80fd\u4e3a\u7a7a\u3002"));
        return;
    }

    User authenticatedUser;
    if (m_repo->checkLogin(username, password, authenticatedUser)) {
        bool userFoundInDb = false;
        for (const auto& u : m_repo->getAllUsers()) {
            if (u.getUsername() == username) {
                m_loggedInUser = u;
                userFoundInDb = true;
                break;
            }
        }

        if (!userFoundInDb) {
            m_loggedInUser = authenticatedUser;
        }

        qDebug() << "[login] success, id:" << m_loggedInUser.getUserId()
                 << "role:" << static_cast<int>(m_loggedInUser.getRole());
        accept();
    } else {
        QMessageBox::warning(this,
                             QStringLiteral("\u767b\u5f55\u5931\u8d25"),
                             QStringLiteral("\u7528\u6237\u540d\u6216\u5bc6\u7801\u65e0\u6548\u3002"));
    }
}
