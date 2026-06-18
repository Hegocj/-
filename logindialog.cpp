#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>

LoginDialog::LoginDialog(ICustomerRepository* repo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_repo(repo)
{
    ui->setupUi(this);

    // 1. 设置登录窗口的初始大小与标题
    this->resize(400, 300);
    this->setWindowTitle("系统登录安全验证");

    // 2. 【安全修正】：不盲目 delete 布局，如果 UI 文件自带布局，我们直接取出来或者通过重置解决。
    // 为了稳妥，我们直接利用代码重新构建 global 主布局。
    // 确保组件的 parent 指针依然是 this 或跟随新的容器。

    // 3. 核心：创建正中间的“表单+按钮”核心容器
    QWidget* centerContainer = new QWidget(this);
    centerContainer->setFixedWidth(280);

    QVBoxLayout* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(15);

    // 4. 网格布局：对齐标签和输入框
    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(10);

    // 强制让标签固定宽度
    ui->label_username->setFixedWidth(55);
    ui->label_userpassword->setFixedWidth(55);
    ui->label_username->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_userpassword->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 【避坑】：重新声明父对象关系，防止它们由于 UI 文件的旧布局销毁而解绑
    ui->label_username->setParent(centerContainer);
    ui->usernameEdit->setParent(centerContainer);
    ui->label_userpassword->setParent(centerContainer);
    ui->passwordEdit->setParent(centerContainer);
    ui->loginButton->setParent(centerContainer);

    // 把 UI 里的控件塞进网格布局
    formLayout->addWidget(ui->label_username, 0, 0);
    formLayout->addWidget(ui->usernameEdit, 0, 1);
    formLayout->addWidget(ui->label_userpassword, 1, 0);
    formLayout->addWidget(ui->passwordEdit, 1, 1);

    // 5. 组合核心区域
    centerLayout->addLayout(formLayout);

    ui->loginButton->setFixedHeight(35);
    ui->loginButton->setText("安全登录");
    centerLayout->addWidget(ui->loginButton);

    // 6. 终极居中大法：全局用四个大弹簧把 centerContainer 夹在正中间
    QHBoxLayout* globalHorizontalLayout = new QHBoxLayout();
    globalHorizontalLayout->addStretch(1);
    globalHorizontalLayout->addWidget(centerContainer);
    globalHorizontalLayout->addStretch(1);

    // 如果发现已有布局，先解除绑定
    if (this->layout()) {
        delete this->layout();
    }

    QVBoxLayout* globalMainLayout = new QVBoxLayout(this);
    globalMainLayout->addStretch(1);
    globalMainLayout->addLayout(globalHorizontalLayout);
    globalMainLayout->addStretch(1);

    // 7. 【核心修正】：显式强行绑定信号槽，彻底废除 on_loginButton_clicked 自动关联
    // 防止因为重组布局导致 Qt 的 connectSlotsByName 失效
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::handleLoginSubmit);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

// 彻底改为显式响应函数
void LoginDialog::handleLoginSubmit() {
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Prompt", "Username or password cannot be empty.");
        return;
    }

    // 1. 建立临时实体，先通过底层仓库的账户密码鉴权
    User authenticatedUser;
    if (m_repo->checkLogin(username, password, authenticatedUser)) {

        // 【核心修正：去 repo 里面把真正的 user 拿出来】
        // 鉴权通过后，不能直接草率退出！我们要遍历用户库，根据用户名捞出这个人在系统里的真身
        std::vector<User> allSystemUsers = m_repo->getAllUsers();
        bool userFoundInDb = false;

        for (const auto& u : allSystemUsers) {
            // 通过用户名匹配（实际生产环境建议用账号或唯一工号匹配）
            if (u.getUsername() == username) {
                m_loggedInUser = u; // 拿到了仓库里拥有完整 Role, Department, ID 的真实User实体
                userFoundInDb = true;
                break;
            }
        }

        // 容错安全拦截：如果在全量表没捞到（虽然 checkLogin 过了），则退而求其次使用鉴权回填的实体
        if (!userFoundInDb) {
            m_loggedInUser = authenticatedUser;
        }

        qDebug() << "[Login Verified] Successfully retrieved full entity from Repository.";
        qDebug() << " > ID:" << m_loggedInUser.getUserId();
        qDebug() << " > Name:" << m_loggedInUser.getUsername();
        qDebug() << " > Dept:" << m_loggedInUser.getDepartment();
        qDebug() << " > Role Enum:" << static_cast<int>(m_loggedInUser.getRole()); // 此时必然是真实的 Manager 2 或 Admin 3

        accept(); // 返回 QDialog::Accepted，通知 main.cpp 启动分支分流
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}