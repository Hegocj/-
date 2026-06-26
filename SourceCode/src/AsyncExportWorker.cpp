/**
 * @file AsyncExportWorker.cpp
 * @brief 客户数据异步导出任务的具体实现。
 *
 * 研究定位：本文件实现后台导出流程，将界面层传入的 Customer 列表写入文本或
 * Markdown 文件。该设计把耗时 IO 从主线程移出，提高界面响应性。
 *
 * 主要职责：校验导出路径、创建输出文件、按表格格式写入客户编号、姓名、电话、
 * 等级、最后跟进时间和负责人，并通过 Qt 信号报告执行结果。
 */
#include "AsyncExportWorker.h"

#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>

#include <utility>

AsyncExportWorker::AsyncExportWorker(std::vector<Customer> customers, const QString& filePath)
    : m_customers(std::move(customers))
    , m_filePath(filePath)
{
    setAutoDelete(false);
}

void AsyncExportWorker::run()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFailed(QStringLiteral("无法打开导出文件：%1").arg(m_filePath));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    auto cleanCell = [](QString value) {
        value.replace(QStringLiteral("\r"), QStringLiteral(" "));
        value.replace(QStringLiteral("\n"), QStringLiteral(" "));
        return value.trimmed();
    };

    if (QFileInfo(m_filePath).suffix().compare(QStringLiteral("md"), Qt::CaseInsensitive) == 0) {
        out << "| 客户编号 | 客户姓名 | 联系电话 | 客户等级 | 最后跟进时间 | 所属销售ID |\n";
        out << "| --- | --- | --- | --- | --- | --- |\n";
        for (const auto& customer : m_customers) {
            out << "| " << cleanCell(customer.getId())
                << " | " << cleanCell(customer.getName())
                << " | " << cleanCell(customer.getPhone())
                << " | " << cleanCell(customer.getLevel())
                << " | " << cleanCell(customer.getLastFollowTime().toString(Qt::ISODate))
                << " | " << cleanCell(customer.getOwnerId())
                << " |\n";
        }
    } else {
        out << "客户编号\t客户姓名\t联系电话\t客户等级\t最后跟进时间\t所属销售ID\n";
        for (const auto& customer : m_customers) {
            out << cleanCell(customer.getId()) << '\t'
                << cleanCell(customer.getName()) << '\t'
                << cleanCell(customer.getPhone()) << '\t'
                << cleanCell(customer.getLevel()) << '\t'
                << cleanCell(customer.getLastFollowTime().toString(Qt::ISODate)) << '\t'
                << cleanCell(customer.getOwnerId()) << '\n';
        }
    }

    emit exportFinished(m_filePath, static_cast<int>(m_customers.size()));
}
