#include "DataMockGenerator.h"

#include <QDateTime>
#include <QRandomGenerator>
#include <QtGlobal>

std::vector<Customer> DataMockGenerator::generateCustomers(int count, const QStringList& ownerIds)
{
    static const QStringList surnames = {
        QStringLiteral("Zhang"), QStringLiteral("Li"), QStringLiteral("Wang"),
        QStringLiteral("Chen"), QStringLiteral("Liu"), QStringLiteral("Zhao")
    };
    static const QStringList givenNames = {
        QStringLiteral("Ming"), QStringLiteral("Lei"), QStringLiteral("Fang"),
        QStringLiteral("Na"), QStringLiteral("Jun"), QStringLiteral("Qiang")
    };
    static const QStringList levels = {
        QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("VIP"), QStringLiteral("Normal")
    };

    std::vector<Customer> customers;
    customers.reserve(static_cast<size_t>(qMax(0, count)));

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < count; ++i) {
        const int seed = static_cast<int>(QRandomGenerator::global()->bounded(100000, 999999));
        const QString id = QStringLiteral("MC%1").arg(now + i);
        const QString name = surnames.at(seed % surnames.size()) + QStringLiteral(" ") +
                             givenNames.at((seed / 10) % givenNames.size());
        const QString phone = QStringLiteral("13%1").arg(seed * 1000 + i, 9, 10, QLatin1Char('0')).right(11);
        const QString level = levels.at(seed % levels.size());
        const QDateTime lastFollow = QDateTime::currentDateTime().addDays(-static_cast<int>(QRandomGenerator::global()->bounded(0, 90)));
        const QString owner = ownerIds.isEmpty() ? QString() : ownerIds.at(seed % ownerIds.size());

        customers.emplace_back(id, name, phone, level, lastFollow, owner);
    }

    return customers;
}

bool DataMockGenerator::appendToRepository(ICustomerRepository& repository, int count, const QStringList& ownerIds)
{
    const auto customers = generateCustomers(count, ownerIds);
    for (const auto& customer : customers) {
        if (!repository.saveCustomer(customer)) {
            return false;
        }
    }
    return true;
}
