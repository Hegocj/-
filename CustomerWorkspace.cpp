#include "CustomerWorkspace.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

CustomerWorkspace::CustomerWorkspace(CustomerController* controller, QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
{
    auto* rootLayout = new QVBoxLayout(this);
    auto* toolbarLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("\u641c\u7d22\u5ba2\u6237\u59d3\u540d\u6216\u7535\u8bdd"));
    m_refreshButton = new QPushButton(QStringLiteral("\u5237\u65b0"), this);
    toolbarLayout->addWidget(m_searchEdit, 1);
    toolbarLayout->addWidget(m_refreshButton);
    rootLayout->addLayout(toolbarLayout);

    m_customerTable = new QTableWidget(this);
    m_customerTable->setColumnCount(6);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5ba2\u6237\u7f16\u53f7"),
        QStringLiteral("\u5ba2\u6237\u59d3\u540d"),
        QStringLiteral("\u8054\u7cfb\u7535\u8bdd"),
        QStringLiteral("\u5ba2\u6237\u7b49\u7ea7"),
        QStringLiteral("\u6700\u540e\u8ddf\u8fdb\u65f6\u95f4"),
        QStringLiteral("\u6240\u5c5e\u9500\u552eID")
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
        const bool isVip = customer.getLevel().compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0;
        auto* idItem = new QTableWidgetItem(customer.getId());
        auto* nameItem = new QTableWidgetItem(isVip
                                                  ? QStringLiteral("[VIP] %1").arg(customer.getName())
                                                  : customer.getName());
        auto* phoneItem = new QTableWidgetItem(customer.getPhone());
        auto* levelItem = new QTableWidgetItem(isVip ? QStringLiteral("VIP") : QStringLiteral("\u666e\u901a"));
        auto* followItem = new QTableWidgetItem(customer.getLastFollowTime().toString(QStringLiteral("yyyy-MM-dd HH:mm")));
        auto* ownerItem = new QTableWidgetItem(customer.getOwnerId());
        if (isVip) {
            const QBrush vipBrush(QColor(255, 245, 210));
            for (auto* item : {idItem, nameItem, phoneItem, levelItem, followItem, ownerItem}) {
                item->setBackground(vipBrush);
                item->setForeground(QBrush(QColor(128, 78, 0)));
            }
            levelItem->setToolTip(QStringLiteral("VIP\u5ba2\u6237\uff0c\u5df2\u7f6e\u9876\u663e\u793a"));
        }
        m_customerTable->setItem(row, 0, idItem);
        m_customerTable->setItem(row, 1, nameItem);
        m_customerTable->setItem(row, 2, phoneItem);
        m_customerTable->setItem(row, 3, levelItem);
        m_customerTable->setItem(row, 4, followItem);
        m_customerTable->setItem(row, 5, ownerItem);
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
