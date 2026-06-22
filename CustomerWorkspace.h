#ifndef CUSTOMERWORKSPACE_H
#define CUSTOMERWORKSPACE_H

#include "CustomerController.h"
#include "TimelineWidget.h"

#include <QWidget>
#include <vector>

class QLineEdit;
class QPushButton;
class QTableWidget;

class CustomerWorkspace : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerWorkspace(CustomerController* controller, QWidget* parent = nullptr);

private slots:
    void renderCustomers(const std::vector<Customer>& customers);
    void renderFollowRecords(const QString& customerId, const std::vector<FollowRecord>& records);
    void handleSelectionChanged();

private:
    CustomerController* m_controller;
    QLineEdit* m_searchEdit;
    QPushButton* m_refreshButton;
    QTableWidget* m_customerTable;
    TimelineWidget* m_timelineWidget;
    std::vector<Customer> m_customers;
};

#endif // CUSTOMERWORKSPACE_H
