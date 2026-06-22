#include "SalesMainWindow.h"
#include "customerdetaildialog.h"

#include <QBrush>
#include <QColor>
#include <QDialog>
#include <QTableWidgetItem>

SalesMainWindow::SalesMainWindow(std::shared_ptr<ICustomerRepository> repo, const User& user, QWidget *parent)
    : BaseMainWindow(repo, user, parent)
{
}

void SalesMainWindow::initRoleMenu()
{
    m_leftMenu->clear();
    m_leftMenu->addItems({
        QStringLiteral("\u6211\u7684\u5ba2\u6237"),
        QStringLiteral("\u7cfb\u7edf\u516c\u6d77\u6c60")
    });
}

void SalesMainWindow::refreshDataByMenu(int index)
{
    m_searchEdit->clear();

    if (index == 0) {
        m_displayedCustomers = m_repo->getCustomersBySales(m_currentUser.getUserId());
    } else if (index == 1) {
        m_displayedCustomers = m_repo->getHighSeasCustomers();
    }

    renderTableData(m_displayedCustomers);
}

void SalesMainWindow::executeSearch(const QString& key)
{
    if (key.isEmpty()) {
        refreshDataByMenu(m_leftMenu->currentRow());
        return;
    }

    m_displayedCustomers = m_repo->searchCustomers(key, m_currentUser);
    renderTableData(m_displayedCustomers);
}

void SalesMainWindow::executeRowModification(int row)
{
    if (row < 0 || row >= static_cast<int>(m_displayedCustomers.size())) {
        return;
    }

    CustomerDetailDialog detailDlg(m_displayedCustomers[row].getId(), m_repo, m_currentUser, this);
    if (detailDlg.exec() == QDialog::Accepted) {
        refreshDataByMenu(m_leftMenu->currentRow());
    }
}

void SalesMainWindow::renderTableData(const std::vector<Customer>& customers)
{
    m_customerTable->setRowCount(0);
    m_customerTable->setColumnCount(4);
    m_customerTable->setHorizontalHeaderLabels({
        QStringLiteral("\u5ba2\u6237ID"),
        QStringLiteral("\u5ba2\u6237\u59d3\u540d"),
        QStringLiteral("\u8054\u7cfb\u7535\u8bdd"),
        QStringLiteral("\u7b49\u7ea7/\u72b6\u6001")
    });

    for (int row = 0; row < static_cast<int>(customers.size()); ++row) {
        const Customer& customer = customers.at(static_cast<size_t>(row));
        m_customerTable->insertRow(row);
        const bool isVip = customer.getLevel().compare(QStringLiteral("VIP"), Qt::CaseInsensitive) == 0;
        auto* idItem = new QTableWidgetItem(customer.getId());
        auto* nameItem = new QTableWidgetItem(isVip
                                                  ? QStringLiteral("[VIP] %1").arg(customer.getName())
                                                  : customer.getName());
        auto* phoneItem = new QTableWidgetItem(customer.getPhone());

        auto* levelItem = new QTableWidgetItem(customer.getOwnerId().isEmpty()
                                                   ? QStringLiteral("\u516c\u6d77\u6c60 - %1").arg(isVip ? QStringLiteral("VIP") : QStringLiteral("\u666e\u901a"))
                                                   : (isVip ? QStringLiteral("VIP") : QStringLiteral("\u666e\u901a")));
        if (customer.getOwnerId().isEmpty()) {
            levelItem->setForeground(QBrush(Qt::darkYellow));
        }
        if (isVip) {
            const QBrush vipBrush(QColor(255, 245, 210));
            for (auto* item : {idItem, nameItem, phoneItem, levelItem}) {
                item->setBackground(vipBrush);
                item->setForeground(QBrush(QColor(128, 78, 0)));
            }
            levelItem->setToolTip(QStringLiteral("VIP\u5ba2\u6237\uff0c\u5df2\u7f6e\u9876\u663e\u793a"));
        }
        m_customerTable->setItem(row, 0, idItem);
        m_customerTable->setItem(row, 1, nameItem);
        m_customerTable->setItem(row, 2, phoneItem);
        m_customerTable->setItem(row, 3, levelItem);
    }
}
