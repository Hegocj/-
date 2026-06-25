/**
 * @file basemainwindow.cpp
 * @brief 三类角色主窗口公共行为的实现。
 *
 * 研究定位：本文件实现主窗口框架层逻辑，负责把 Qt Designer 中的 mainPlaceholder
 * 改造成左侧菜单加右侧表格的业务工作区。它把通用交互沉淀到基类，减少各角色窗口
 * 的重复代码。
 *
 * 主要职责：构建主界面布局，绑定搜索、防抖、菜单切换、双击行、导入、导出和登出
 * 信号；根据当前用户角色导出可见范围内的客户数据；为子类提供跟进时间线弹窗入口。
 *
 * 编码说明：本文件中形如 "\u4e2d\u6587" 的内容是 Unicode 转义字符串，
 * 编译运行后会显示为中文，用来避免不同编辑器编码设置导致界面文字乱码。
 */
#include "BaseMainWindow.h"
#include "AsyncExportWorker.h"
#include "followtimelinedialog.h"
#include "ui_mainwindow.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStringConverter>
#include <QThreadPool>
#include <QTextStream>
#include <QVBoxLayout>

#include <set>
#include <utility>

BaseMainWindow::BaseMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_repo(std::move(repo))
    , m_currentUser(user)
{
    ui->setupUi(this);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);

    initMixLayout();
    bindCoreSignals();
}

BaseMainWindow::~BaseMainWindow()
{
    delete ui;
}

void BaseMainWindow::bootstrapMainWindow()
{
    initRoleMenu();
    if (m_leftMenu && m_leftMenu->count() > 0) {
        m_leftMenu->setCurrentRow(0);
    }
}

void BaseMainWindow::initMixLayout()
{
    setWindowTitle(QStringLiteral("\u5ba2\u6237\u4fe1\u606f\u7ba1\u7406\u7cfb\u7edf - \u5f53\u524d\u7528\u6237\uff1a%1").arg(m_currentUser.getUsername()));
    setMinimumSize(900, 560);

    if (!ui->mainPlaceholder) {
        QMessageBox::critical(this, QStringLiteral("UI Error"),
                              QStringLiteral("MainWindow.ui requires a widget named mainPlaceholder."));
        return;
    }

    auto* placeholderLayout = new QVBoxLayout(ui->mainPlaceholder);
    placeholderLayout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, ui->mainPlaceholder);
    placeholderLayout->addWidget(m_splitter);

    m_leftMenu = new QListWidget(m_splitter);
    m_leftMenu->setFixedWidth(200);
    m_leftMenu->setMinimumWidth(150);

    m_rightContainer = new QWidget(m_splitter);
    auto* rightLayout = new QVBoxLayout(m_rightContainer);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(8);

    auto* searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(m_rightContainer);
    m_searchEdit->setPlaceholderText(QStringLiteral("\u8bf7\u8f93\u5165\u5ba2\u6237\u59d3\u540d\u3001\u7535\u8bdd\u6216\u5ba2\u6237\u7f16\u53f7\u641c\u7d22"));
    m_searchButton = new QPushButton(QStringLiteral("\u641c\u7d22"), m_rightContainer);
    m_importBtn = new QPushButton(QStringLiteral("\u5bfc\u5165"), m_rightContainer);
    m_exportBtn = new QPushButton(QStringLiteral("\u5bfc\u51fa"), m_rightContainer);
    m_logoutBtn = new QPushButton(QStringLiteral("\u767b\u51fa"), m_rightContainer);

    searchLayout->addWidget(m_searchEdit, 1);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addSpacing(12);
    searchLayout->addWidget(m_importBtn);
    searchLayout->addWidget(m_exportBtn);
    searchLayout->addSpacing(12);
    searchLayout->addWidget(m_logoutBtn);
    rightLayout->addLayout(searchLayout);

    m_customerTable = new QTableWidget(m_rightContainer);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5ba2\u6237ID"),
        QStringLiteral("\u5ba2\u6237\u59d3\u540d"),
        QStringLiteral("\u8054\u7cfb\u7535\u8bdd"),
        QStringLiteral("\u72b6\u6001/\u7b49\u7ea7")
    });
    m_customerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_customerTable->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    rightLayout->addWidget(m_customerTable, 1);

    m_splitter->addWidget(m_leftMenu);
    m_splitter->addWidget(m_rightContainer);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 4);
}

void BaseMainWindow::bindCoreSignals()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BaseMainWindow::onSearchTextChanged);
    connect(m_debounceTimer, &QTimer::timeout, this, &BaseMainWindow::onDebounceTimerTimeout);
    connect(m_searchButton, &QPushButton::clicked, this, &BaseMainWindow::onDebounceTimerTimeout);
    connect(m_leftMenu, &QListWidget::currentRowChanged, this, &BaseMainWindow::onMenuIndexChanged);
    connect(m_customerTable, &QTableWidget::cellDoubleClicked, this, &BaseMainWindow::onTableDoubleClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, [this]() {
        emit logoutRequested();
        close();
    });

    connect(m_importBtn, &QPushButton::clicked, this, [this]() {
        if (!m_repo) {
            QMessageBox::warning(this, QStringLiteral("\u5bfc\u5165"), QStringLiteral("\u6570\u636e\u4ed3\u5e93\u4e0d\u53ef\u7528\u3002"));
            return;
        }

        const QString filePath = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("\u5bfc\u5165\u5ba2\u6237\u6570\u636e"),
            QString(),
            QStringLiteral("\u6587\u672c\u6216 Markdown \u6587\u4ef6 (*.txt *.md)")
        );
        if (filePath.isEmpty()) {
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, QStringLiteral("\u5bfc\u5165"), QStringLiteral("\u65e0\u6cd5\u6253\u5f00\u5bfc\u5165\u6587\u4ef6\u3002"));
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        int importedCount = 0;
        int skippedCount = 0;
        std::set<QString> existingCustomerIds;
        for (const auto& customer : m_repo->getAllCustomers()) {
            existingCustomerIds.insert(customer.getId());
        }
        std::set<QString> importedCustomerIds;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            QStringList fields;
            if (line.startsWith(QLatin1Char('|'))) {
                if (line.contains(QStringLiteral("---")) ||
                    line.contains(QStringLiteral("\u5ba2\u6237\u7f16\u53f7")) ||
                    line.contains(QStringLiteral("id"), Qt::CaseInsensitive)) {
                    continue;
                }
                line.remove(0, 1);
                if (line.endsWith(QLatin1Char('|'))) {
                    line.chop(1);
                }
                fields = line.split(QLatin1Char('|'));
            } else {
                if (line.startsWith(QStringLiteral("\u5ba2\u6237\u7f16\u53f7")) ||
                    line.startsWith(QStringLiteral("id"), Qt::CaseInsensitive)) {
                    continue;
                }
                fields = line.split(QLatin1Char('\t'));
            }

            for (QString& field : fields) {
                field = field.trimmed();
            }

            if (fields.size() < 4) {
                ++skippedCount;
                continue;
            }

            const QString rawCustomerId = fields.value(0).trimmed();
            if (!rawCustomerId.isEmpty() &&
                (existingCustomerIds.find(rawCustomerId) != existingCustomerIds.end() ||
                 importedCustomerIds.find(rawCustomerId) != importedCustomerIds.end())) {
                ++skippedCount;
                continue;
            }

            Customer customer;
            customer.setId(rawCustomerId);
            customer.setName(fields.value(1).trimmed());
            customer.setPhone(fields.value(2).trimmed());
            customer.setLevel(fields.value(3).trimmed());

            QDateTime lastFollowTime = QDateTime::fromString(fields.value(4).trimmed(), Qt::ISODate);
            if (!lastFollowTime.isValid()) {
                lastFollowTime = QDateTime::currentDateTime();
            }
            customer.setLastFollowTime(lastFollowTime);
            customer.setOwnerId(fields.value(5).trimmed());

            if (customer.getId().isEmpty()) {
                customer.setId(QStringLiteral("IMP_%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(importedCount));
            }

            if (existingCustomerIds.find(customer.getId()) != existingCustomerIds.end() ||
                importedCustomerIds.find(customer.getId()) != importedCustomerIds.end()) {
                ++skippedCount;
                continue;
            }

            if (!customer.isValid() || !m_repo->saveCustomer(customer)) {
                ++skippedCount;
                continue;
            }
            importedCustomerIds.insert(customer.getId());
            ++importedCount;
        }

        QMessageBox::information(this, QStringLiteral("\u5bfc\u5165\u5b8c\u6210"),
                                 QStringLiteral("\u6210\u529f\u5bfc\u5165 %1 \u6761\u5ba2\u6237\u6570\u636e\uff0c\u8df3\u8fc7 %2 \u6761\u3002")
                                     .arg(importedCount)
                                     .arg(skippedCount));
        if (m_leftMenu->currentRow() >= 0) {
            refreshDataByMenu(m_leftMenu->currentRow());
        }
    });

    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (!m_repo) {
            QMessageBox::warning(this, QStringLiteral("\u5bfc\u51fa"), QStringLiteral("\u6570\u636e\u4ed3\u5e93\u4e0d\u53ef\u7528\u3002"));
            return;
        }

        const QString filePath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("\u5bfc\u51fa\u5ba2\u6237\u6570\u636e"),
            QStringLiteral("\u5ba2\u6237\u6570\u636e.txt"),
            QStringLiteral("\u6587\u672c\u6216 Markdown \u6587\u4ef6 (*.txt *.md)")
        );
        if (filePath.isEmpty()) {
            return;
        }

        std::vector<Customer> customers;
        switch (m_currentUser.getRole()) {
        case UserRole::Sales:
            customers = m_repo->getCustomersBySales(m_currentUser.getUserId());
            break;
        case UserRole::Manager:
            customers = m_repo->getCustomersByDepartment(m_currentUser.getDepartment());
            break;
        case UserRole::Admin:
            customers = m_repo->getAllCustomers();
            break;
        }

        auto* worker = new AsyncExportWorker(customers, filePath);
        connect(worker, &AsyncExportWorker::exportFinished, this, [this, worker](const QString& path, int rowCount) {
            worker->deleteLater();
            QMessageBox::information(this, QStringLiteral("\u5bfc\u51fa\u5b8c\u6210"),
                                     QStringLiteral("\u5df2\u5bfc\u51fa %1 \u6761\u5ba2\u6237\u6570\u636e\u5230\uff1a%2").arg(rowCount).arg(path));
        }, Qt::QueuedConnection);
        connect(worker, &AsyncExportWorker::exportFailed, this, [this, worker](const QString& message) {
            worker->deleteLater();
            QMessageBox::warning(this, QStringLiteral("\u5bfc\u51fa\u5931\u8d25"), message);
        }, Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(worker);
    });
}

void BaseMainWindow::onSearchTextChanged(const QString& text)
{
    m_pendingSearchKey = text;
    m_debounceTimer->start(300);
}

void BaseMainWindow::onDebounceTimerTimeout()
{
    m_debounceTimer->stop();
    executeSearch(m_searchEdit->text().trimmed());
}

void BaseMainWindow::onMenuIndexChanged(int index)
{
    if (index >= 0) {
        refreshDataByMenu(index);
    }
}

void BaseMainWindow::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0) {
        executeRowModification(row);
    }
}

bool BaseMainWindow::showFollowTimelineDialog(const QString& customerId, bool enableAssignButton)
{
    Q_UNUSED(enableAssignButton);

    FollowTimelineDialog dialog(customerId, m_repo, m_currentUser, this);
    return dialog.exec() == QDialog::Accepted;
}
