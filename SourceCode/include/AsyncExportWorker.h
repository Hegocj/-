/**
 * @file AsyncExportWorker.h
 * @brief 客户数据异步导出任务的类声明。
 *
 * 研究定位：本文件属于基础支撑层中的后台任务模块，用于避免大量客户数据导出时阻塞
 * Qt 主界面线程。它通过 QObject 信号向界面层回传导出结果。
 *
 * 主要职责：保存待导出的客户集合和目标文件路径，声明 QRunnable::run() 执行入口，
 * 并定义导出成功、导出失败两个信号，供 BaseMainWindow 监听。
 */
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
