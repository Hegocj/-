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
    setWindowTitle(QStringLiteral("客户信息管理系统 - 当前用户：%1").arg(m_currentUser.getUsername()));
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
    m_searchEdit->setPlaceholderText(QStringLiteral("请输入客户姓名、电话或员工编号搜索"));
    m_searchButton = new QPushButton(QStringLiteral("搜索"), m_rightContainer);
    m_importBtn = new QPushButton(QStringLiteral("导入"), m_rightContainer);
    m_exportBtn = new QPushButton(QStringLiteral("导出"), m_rightContainer);

    searchLayout->addWidget(m_searchEdit, 1);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addSpacing(12);
    searchLayout->addWidget(m_importBtn);
    searchLayout->addWidget(m_exportBtn);
    rightLayout->addLayout(searchLayout);

    m_customerTable = new QTableWidget(m_rightContainer);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Name"),
        QStringLiteral("Phone"),
        QStringLiteral("Status / Level")
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
    connect(m_importBtn, &QPushButton::clicked, this, [this]() {
        if (!m_repo) {
            QMessageBox::warning(this, QStringLiteral("导入"), QStringLiteral("数据仓储不可用。"));
            return;
        }

        const QString filePath = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("导入客户数据"),
            QString(),
            QStringLiteral("文本或 Markdown 文件 (*.txt *.md)")
        );
        if (filePath.isEmpty()) {
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, QStringLiteral("导入"), QStringLiteral("无法打开导入文件。"));
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        int importedCount = 0;
        int skippedCount = 0;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }

            QStringList fields;
            if (line.startsWith(QLatin1Char('|'))) {
                if (line.contains(QStringLiteral("---")) || line.contains(QStringLiteral("客户编号"))) {
                    continue;
                }
                line.remove(0, 1);
                if (line.endsWith(QLatin1Char('|'))) {
                    line.chop(1);
                }
                fields = line.split(QLatin1Char('|'));
            } else {
                if (line.startsWith(QStringLiteral("客户编号"))) {
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

            Customer customer;
            customer.setId(fields.value(0).trimmed());
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

            if (!customer.isValid() || !m_repo->saveCustomer(customer)) {
                ++skippedCount;
                continue;
            }
            ++importedCount;
        }

        QMessageBox::information(this, QStringLiteral("导入完成"),
                                 QStringLiteral("成功导入 %1 条客户数据，跳过 %2 条。")
                                     .arg(importedCount)
                                     .arg(skippedCount));
        if (m_leftMenu->currentRow() >= 0) {
            refreshDataByMenu(m_leftMenu->currentRow());
        }
    });
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (!m_repo) {
            QMessageBox::warning(this, QStringLiteral("导出"), QStringLiteral("数据仓储不可用。"));
            return;
        }

        const QString filePath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("导出客户数据"),
            QStringLiteral("客户数据.txt"),
            QStringLiteral("文本或 Markdown 文件 (*.txt *.md)")
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
            QMessageBox::information(this, QStringLiteral("导出完成"),
                                     QStringLiteral("已导出 %1 条客户数据到：%2").arg(rowCount).arg(path));
        }, Qt::QueuedConnection);
        connect(worker, &AsyncExportWorker::exportFailed, this, [this, worker](const QString& message) {
            worker->deleteLater();
            QMessageBox::warning(this, QStringLiteral("导出失败"), message);
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
