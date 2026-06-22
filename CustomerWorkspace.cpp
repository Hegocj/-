#include "CustomerWorkspace.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

CustomerWorkspace::CustomerWorkspace(CustomerController* controller, QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* toolbarLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("搜索客户姓名或电话"));
    m_refreshButton = new QPushButton(QStringLiteral("刷新"), this);
    toolbarLayout->addWidget(m_searchEdit, 1);
    toolbarLayout->addWidget(m_refreshButton);
    rootLayout->addLayout(toolbarLayout);

    m_customerTable = new QTableWidget(this);
    m_customerTable->setColumnCount(6);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("客户编号"),
        QStringLiteral("客户姓名"),
        QStringLiteral("联系电话"),
        QStringLiteral("客户等级"),
        QStringLiteral("最后跟进时间"),
        QStringLiteral("所属销售ID")
    });
    m_customerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rootLayout->addWidget(m_customerTable, 2);

    m_timelineWidget = new TimelineWidget(this);
    rootLayout->addWidget(m_timelineWidget, 1);

    if (m_controller) {
        connect(m_searchEdit, &QLineEdit::textChanged, m_controller, &CustomerController::searchCustomers);
        connect(m_refreshButton, &QPushButton::clicked, m_controller, &CustomerController::loadCustomers);
        connect(m_controller, &CustomerController::customersLoaded, this, &CustomerWorkspace::renderCustomers);
        connect(m_controller, &CustomerController::followRecordsLoaded, this, &CustomerWorkspace::renderFollowRecords);
        connect(m_customerTable, &QTableWidget::itemSelectionChanged, this, &CustomerWorkspace::handleSelectionChanged);
        m_controller->loadCustomers();
    }
}

void CustomerWorkspace::renderCustomers(const std::vector<Customer>& customers)
{
    m_customers = customers;
    m_customerTable->setRowCount(0);

    for (int row = 0; row < static_cast<int>(m_customers.size()); ++row) {
        const auto& customer = m_customers.at(static_cast<size_t>(row));
        m_customerTable->insertRow(row);
        m_customerTable->setItem(row, 0, new QTableWidgetItem(customer.getId()));
        m_customerTable->setItem(row, 1, new QTableWidgetItem(customer.getName()));
        m_customerTable->setItem(row, 2, new QTableWidgetItem(customer.getPhone()));
        m_customerTable->setItem(row, 3, new QTableWidgetItem(customer.getLevel()));
        m_customerTable->setItem(row, 4, new QTableWidgetItem(customer.getLastFollowTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"))));
        m_customerTable->setItem(row, 5, new QTableWidgetItem(customer.getOwnerId()));
    }
}

void CustomerWorkspace::renderFollowRecords(const QString& customerId, const std::vector<FollowRecord>& records)
{
    Q_UNUSED(customerId);
    m_timelineWidget->setRecords(records);
}

void CustomerWorkspace::handleSelectionChanged()
{
    if (!m_controller) {
        return;
    }

    const int row = m_customerTable->currentRow();
    if (row < 0 || row >= static_cast<int>(m_customers.size())) {
        m_timelineWidget->setRecords({});
        return;
    }

    m_controller->loadFollowRecords(m_customers.at(static_cast<size_t>(row)).getId());
}
