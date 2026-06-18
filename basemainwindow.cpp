#include "BaseMainWindow.h"
#include "ui_mainwindow.h" // 确保能拿到 mainwindow.ui 生成的结构体
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
//debug
#include<QDebug>

BaseMainWindow::BaseMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_repo(repo)
    , m_currentUser(user)
{
    ui->setupUi(this); // 1. 先把 ui 的外壳壳子（包含菜单栏、状态栏、mainPlaceholder）建出来

    // 2. 初始化防抖定时器
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true); // 单次触发

    // 3. 核心：用纯代码把精美布局灌入占位符中，绝不碰 centralwidget 及其旧布局！
    initMixLayout();


    // 4. 绑定统一的基础网络/UI事件信号
    bindCoreSignals();

    //5. 捕获双击，什么多余的事都不做，直接把行号扔给虚函数
    connect(m_customerTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        Q_UNUSED(column);

        // 关键：由于 executeRowModification 是纯虚函数，
        // Qt 的多态机制会确保此时调用的是【已经完全构造完毕的子类真身】的函数！
        this->executeRowModification(row);
    });
}

BaseMainWindow::~BaseMainWindow()
{
    delete ui;
}

void BaseMainWindow::bootstrapMainWindow()
{
    //  此时子类对象已经在内存里 100% 完工了！
    // 此时调用纯虚函数，多态机制完全苏醒，绝对安全有用！
    initRoleMenu();

    // 顺手把默认选中第一行的逻辑也在这里统一办了
    if (m_leftMenu && m_leftMenu->count() > 0) {
        m_leftMenu->setCurrentRow(0);
    }
}

void BaseMainWindow::initMixLayout()
{
    // 动态拼接标题，彰显当前权限上下文
    this->setWindowTitle("企业级 CRM 系统 - 当前用户: " + m_currentUser.getUsername());
    this->setMinimumSize(800, 500);

    // 【核心修正点】：如果 ui 里面没有拉占位符容器，抛出安全警告，防止野指针
    if (!ui->mainPlaceholder) {
        QMessageBox::critical(this, "架构错误", "MainWindow.ui 中未找到名为 'mainPlaceholder' 的 QWidget 容器！");
        return;
    }

    // 直接为ui->mainPlaceholder 设置一个全新的水平/垂直布局。
    // 这叫 Sandbox 隔离，里面的东西再怎么翻江倒海，都不会破坏外面 UI 文件拖的其他原生组件。
    QVBoxLayout* placeholderLayout = new QVBoxLayout(ui->mainPlaceholder);
    placeholderLayout->setContentsMargins(0, 0, 0, 0); // 像素级贴边

    // 1. 创建主分裂器，parent 挂在 mainPlaceholder 上
    m_splitter = new QSplitter(Qt::Horizontal, ui->mainPlaceholder);
    placeholderLayout->addWidget(m_splitter);

    // 2. 左侧导航菜单（先留空，子类在 initRoleMenu 里通过 addItems 自行注入权限菜单）
    m_leftMenu = new QListWidget(m_splitter);
    m_leftMenu->setFixedWidth(200);
    m_leftMenu->setMinimumWidth(150);

    // 3. 右侧容器及垂直布局
    m_rightContainer = new QWidget(m_splitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(m_rightContainer);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(8);

    // 4. 右上角精美搜索栏
    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(m_rightContainer);
    m_searchEdit->setPlaceholderText("请输入客户姓名或电话搜索（支持300ms智能防抖）...");
    m_searchButton = new QPushButton("搜索", m_rightContainer);

    searchLayout->addWidget(m_searchEdit, 1);
    searchLayout->addWidget(m_searchButton);
    rightLayout->addLayout(searchLayout);

    // 5. 右下角自带滚轮的精美表格
    m_customerTable = new QTableWidget(m_rightContainer);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({"客户ID", "客户姓名", "联系电话", "跟进状态/级别"});
    m_customerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_customerTable->setVerticalScrollMode(QAbstractItemView::ScrollPerItem); // 丝滑滚动

    rightLayout->addWidget(m_customerTable, 1);

    // 6. 完美的比例组合放入分裂器
    m_splitter->addWidget(m_leftMenu);
    m_splitter->addWidget(m_rightContainer);
    m_splitter->setStretchFactor(0, 1);  // 左侧权重 1
    m_splitter->setStretchFactor(1, 4);  // 右侧权重 4
}

void BaseMainWindow::bindCoreSignals()
{
    // 1. 搜索框打字时：不立刻读库，触发防抖机制
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BaseMainWindow::onSearchTextChanged);

    // 2. 定时器到期：正式调用子类的 executeSearch
    connect(m_debounceTimer, &QTimer::timeout, this, &BaseMainWindow::onDebounceTimerTimeout);

    // 3. 点击传统搜索按钮：立刻打破防抖，直接执行搜索
    connect(m_searchButton, &QPushButton::clicked, this, &BaseMainWindow::onDebounceTimerTimeout);

    // 4. 侧边栏切换：交由统一调度槽，最终流转到子类的 refreshDataByMenu
    connect(m_leftMenu, &QListWidget::currentRowChanged, this, &BaseMainWindow::onMenuIndexChanged);

    // 5. 表格双击：拦截并调用子类的特权行业务行改动
    connect(m_customerTable, &QTableWidget::cellDoubleClicked, this, &BaseMainWindow::onTableDoubleClicked);
}

// =========================================================================
//  核心信号中转调度实现
// =========================================================================

void BaseMainWindow::onSearchTextChanged(const QString& text)
{
    m_pendingSearchKey = text;
    m_debounceTimer->start(300); // 只要在 300ms 内继续打字，此定时器就会被强行刷新，完美保护后端
}

void BaseMainWindow::onDebounceTimerTimeout()
{
    m_debounceTimer->stop();
    // 多态：调用子类填空的 executeSearch，不同角色搜出来的 WHERE 条件天然隔离！
    executeSearch(m_searchEdit->text().trimmed());
}

void BaseMainWindow::onMenuIndexChanged(int index)
{
    if (index < 0) return;
    // 多态：调用子类填空的 refreshDataByMenu
    refreshDataByMenu(index);
}

void BaseMainWindow::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0) return;
    // 多态：调用子类填空的行业务改动
    executeRowModification(row);
}

// 后续由子类或基类完善的时间轴公共弹窗方法
bool BaseMainWindow::showFollowTimelineDialog(const QString& customerId, bool enableAssignButton)
{
    // 暂时占位，后续统一编写具体的时间轴弹窗类
    Q_UNUSED(customerId);
    Q_UNUSED(enableAssignButton);
    return false;
}