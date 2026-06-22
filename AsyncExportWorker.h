#ifndef ASYNCEXPORTWORKER_H
#define ASYNCEXPORTWORKER_H

#include "Customer.h"

#include <QObject>
#include <QRunnable>
#include <QString>
#include <vector>

class AsyncExportWorker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    AsyncExportWorker(std::vector<Customer> customers, const QString& filePath);

    void run() override;

signals:
    void exportFinished(const QString& filePath, int rowCount);
    void exportFailed(const QString& message);

private:
    std::vector<Customer> m_customers;
    QString m_filePath;
};

#endif // ASYNCEXPORTWORKER_H
