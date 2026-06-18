#include "customercarddialog.h"
#include "FollowTimelineDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

CustomerDetailDialog::CustomerDetailDialog(const QString& customerId,
                                           std::shared_ptr<ICustomerRepository> repo,
                                           const User& currentUser,
                                           QWidget *parent)
    : QDialog(parent)
    , m_customerId(customerId)
    , m_repo(repo)
    , m_currentUser(currentUser)
{
    setWindowTitle("Unified Customer Matrix Panel");
    resize(420, 340);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    // 1. 初始化全量表单
    m_idEdit = new QLineEdit(this);    m_idEdit->setReadOnly(true); // 任何人都不能改 ID
    m_nameEdit = new QLineEdit(this);
    m_phoneEdit = new QLineEdit(this);
    m_levelEdit = new QLineEdit(this);

    // 负责人直接采用 ComboBox 承载
    m_ownerCombo = new QComboBox(this);

    formLayout->addRow("Customer ID:", m_idEdit);
    formLayout->addRow("Customer Name:", m_nameEdit);
    formLayout->addRow("Phone Number:", m_phoneEdit);
    formLayout->addRow("Customer Level:", m_levelEdit);
    formLayout->addRow("Assigned Sales Staff:", m_ownerCombo);
    mainLayout->addLayout(formLayout);

    // 2. 注入核心高能“跟进信息”按钮
    m_followBtn = new QPushButton("View & Append Follow-up Timeline", this);
    mainLayout->addWidget(m_followBtn);

    mainLayout->addSpacing(10);

    // 3. 动态拓展功能按钮栏（认领、放逐特权）
    QHBoxLayout* featureLayout = new QHBoxLayout();
    m_claimBtn = new QPushButton("Claim to My Private Sea", this);
    m_evictBtn = new QPushButton("Force Evict to Public High Sea", this);
    m_claimBtn->hide(); // 默认先隐藏，靠权限判定决定是否亮起
    m_evictBtn->hide();
    featureLayout->addWidget(m_claimBtn);
    featureLayout->addWidget(m_evictBtn);
    mainLayout->addLayout(featureLayout);

    // 4. 标准底座按钮（保存/取消）
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton("Save", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    bottomLayout->addWidget(m_saveBtn);
    bottomLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(bottomLayout);

    // 信号绑定
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_followBtn, &QPushButton::clicked, this, &CustomerDetailDialog::openFollowTimeline);
    connect(m_saveBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleSaveOrCommit);
    connect(m_claimBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleClaimAction);
    connect(m_evictBtn, &QPushButton::clicked, this, &CustomerDetailDialog::handleEvictAction);

    // 5. 加载数据并实施严密的动态权限压制
    loadCustomerAndSetupPrivilege();
}

void CustomerDetailDialog::loadCustomerAndSetupPrivilege()
{
    // A. 从数据仓库中提取客户完整实体
    for (const auto& c : m_repo->getAllCustomers()) {
        if (c.getId() == m_customerId) {
            m_customer = c;
            break;
        }
    }

    // 100% 保证信息在所有界面都完整显示出来
    m_idEdit->setText(m_customer.getId());
    m_nameEdit->setText(m_customer.getName());
    m_phoneEdit->setText(m_customer.getPhone());
    m_levelEdit->setText(m_customer.getLevel());

    QString ownerId = m_customer.getOwnerId();

    // =========================================================================
    //  动态权限矩阵重构：改用 setReadOnly 实现“可见不可改”
    // =========================================================================
    if (m_currentUser.getRole() == UserRole::Sales) {

        // 【销售角色判定】
        m_ownerCombo->addItem(ownerId.isEmpty() ? "[None - Public Sea]" : ownerId);
        m_ownerCombo->setEnabled(false); // 销售无法更改负责人下拉框

        if (ownerId.isEmpty()) {
            // 场景 1：销售在【系统公海】双击打开客户
            //  核心修正：全部显示出来，但全部设为只读模式
            m_nameEdit->setReadOnly(true);
            m_phoneEdit->setReadOnly(true);
            m_levelEdit->setReadOnly(true);

            m_followBtn->setEnabled(false); // 未认领前禁止追加跟进
            m_saveBtn->hide();             // 公海池里不需要显示 Save 按钮
            m_claimBtn->show();            // 亮出[认领]特权按钮
        }
        else if (ownerId == m_currentUser.getUserId()) {
            // 场景 2：销售在【我的客户】私海中打开客户
            m_nameEdit->setReadOnly(true);  // 姓名可见，但不能修改
            m_levelEdit->setReadOnly(true); // 级别可见，但不能修改

            m_phoneEdit->setReadOnly(false); //  只有电话解锁只读，销售可以任意修改！
            m_followBtn->setEnabled(true);   //  允许点击查看和追加跟进信息
            m_saveBtn->show();
            m_claimBtn->hide();
        }
        else {
            // 场景 3：极其罕见的防越权安全垫底（看到了别人的客户）
            m_nameEdit->setReadOnly(true);
            m_phoneEdit->setReadOnly(true);
            m_levelEdit->setReadOnly(true);
            m_followBtn->setEnabled(false);
            m_saveBtn->hide();
        }
    }
    else if (m_currentUser.getRole() == UserRole::Manager || m_currentUser.getRole() == UserRole::Admin) {

        // 【经理 / 管理员特权】：全字段可读写
        m_nameEdit->setReadOnly(false);
        m_phoneEdit->setReadOnly(false);
        m_levelEdit->setReadOnly(false);
        m_followBtn->setEnabled(true);
        m_saveBtn->show();

        // 经理专属：洗出旗下销售班底，下拉框完全可用，双击/点击即可一键改派销售
        populateSalesCombo();
        m_ownerCombo->setEnabled(true);

        if (!ownerId.isEmpty()) {
            m_evictBtn->show(); // 只有非公海客户，经理才显示“放逐公海”按钮
        }
    }
}

void CustomerDetailDialog::populateSalesCombo()
{
    m_ownerCombo->clear();
    m_ownerCombo->addItem("[Unassigned / High Sea]", "");

    // 捞出全库用户
    std::vector<User> users = m_repo->getAllUsers();
    int targetIndex = 0;
    int currentIndex = 1;

    for (const auto& u : users) {
        // 如果是管理员，能看到全场所有销售；如果是经理，能看到和自己同属于一个部门的销售
        bool visible = (m_currentUser.getRole() == UserRole::Admin) ||
                       (u.getRole() == UserRole::Sales && u.getDepartment() == m_currentUser.getDepartment());

        if (visible && u.getRole() == UserRole::Sales) {
            // 利用 asprintf 避开 Clazy 警告，规范输出
            QString txt = QString::asprintf("%s (ID: %s)", u.getUsername().toUtf8().constData(), u.getUserId().toUtf8().constData());
            m_ownerCombo->addItem(txt, u.getUserId());

            if (u.getUserId() == m_customer.getOwnerId()) {
                targetIndex = currentIndex; // 锁定制导高亮行
            }
            currentIndex++;
        }
    }
    m_ownerCombo->setCurrentIndex(targetIndex);
}

// =========================================================================
//  3. 业务动作落地
// =========================================================================

void CustomerDetailDialog::handleSaveOrCommit()
{
    // 回写字段
    m_customer.setName(m_nameEdit->text().trimmed());
    m_customer.setPhone(m_phoneEdit->text().trimmed());
    m_customer.setLevel(m_levelEdit->text().trimmed());

    // 如果当前是经理，回写改派后的负责人 ID
    if (m_currentUser.getRole() == UserRole::Manager || m_currentUser.getRole() == UserRole::Admin) {
        m_customer.setOwnerId(m_ownerCombo->currentData().toString());
    }

    if (m_repo->saveCustomer(m_customer)) {
        QMessageBox::information(this, "Notification", "Customer file successfully synchronized.");
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Persistence transmission blocked.");
    }
}

void CustomerDetailDialog::handleClaimAction()
{
    // 公海池直通认领
    if (m_repo->claimCustomer(m_customer.getId(), m_currentUser.getUserId())) {
        QMessageBox::information(this, "Success", "Customer assigned to your private pool.");
        accept();
    } else {
        QMessageBox::warning(this, "Failed", "Claim denied. Asset might have been preempted.");
        reject();
    }
}

void CustomerDetailDialog::handleEvictAction()
{
    // 经理一键剥离，负责人强行刷空置回公海
    auto reply = QMessageBox::question(this, "Eviction Confirmation",
                                       "Are you sure you want to strip this asset and evict it back to the public pool?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_customer.setOwnerId(""); // 清空负责人
        if (m_repo->saveCustomer(m_customer)) {
            QMessageBox::information(this, "Evicted", "Asset returned to high seas.");
            accept();
        }
    }
}

void CustomerDetailDialog::openFollowTimeline()
{
    // 打开时间轴细节舱
    FollowTimelineDialog timelineDlg(m_customerId, m_repo, m_currentUser, this);
    timelineDlg.exec();
}