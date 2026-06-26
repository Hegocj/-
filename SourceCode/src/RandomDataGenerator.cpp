/**
 * @file RandomDataGenerator.cpp
 * @brief 本地演示数据随机生成器实现。
 *
 * 研究定位：本文件实现可重复、可配置的演示数据构造逻辑。它以中文姓名和真实业务字段
 * 填充模型对象，使系统首次启动时可以直接形成较完整的客户管理样例数据。
 *
 * 主要职责：生成管理员、经理、销售账号；按照销售账号分配客户负责人；为部分客户生成
 * 跟进记录。所有账号默认密码由仓库层写入，生成器只负责业务实体内容。
 */
#include "RandomDataGenerator.h"

#include <QDateTime>
#include <QStringList>
#include <QtGlobal>

namespace {

QString paddedId(const QString& prefix, int number, int width)
{
    return QStringLiteral("%1%2").arg(prefix).arg(number, width, 10, QLatin1Char('0'));
}

QString pickName(const QStringList& surnames, const QStringList& givenNames, int index)
{
    return surnames.at(index % surnames.size()) + givenNames.at((index * 7) % givenNames.size());
}

} // namespace

RandomDataSet RandomDataGenerator::generate(const RandomDataConfig& config)
{
    RandomDataSet dataSet;

    const int adminCount = qMax(0, config.adminCount);
    const int managerCount = qMax(0, config.managerCount);
    const int salesCount = qMax(0, config.salesCount);
    const int customerCount = qMax(0, config.customerCount);

    const QStringList departments = {
        QStringLiteral("华东销售部"),
        QStringLiteral("华南销售部"),
        QStringLiteral("华北销售部"),
        QStringLiteral("西南销售部"),
        QStringLiteral("数字渠道部"),
        QStringLiteral("企业客户部"),
        QStringLiteral("互联网渠道部"),
        QStringLiteral("重点客户部")
    };
    const QStringList surnames = {
        QStringLiteral("赵"), QStringLiteral("钱"), QStringLiteral("孙"), QStringLiteral("李"),
        QStringLiteral("周"), QStringLiteral("吴"), QStringLiteral("郑"), QStringLiteral("王"),
        QStringLiteral("冯"), QStringLiteral("陈"), QStringLiteral("褚"), QStringLiteral("卫"),
        QStringLiteral("蒋"), QStringLiteral("沈"), QStringLiteral("韩"), QStringLiteral("杨"),
        QStringLiteral("朱"), QStringLiteral("秦"), QStringLiteral("尤"), QStringLiteral("许"),
        QStringLiteral("何"), QStringLiteral("吕"), QStringLiteral("施"), QStringLiteral("张"),
        QStringLiteral("孔"), QStringLiteral("曹"), QStringLiteral("严"), QStringLiteral("华"),
        QStringLiteral("金"), QStringLiteral("魏"), QStringLiteral("陶"), QStringLiteral("姜")
    };
    const QStringList givenNames = {
        QStringLiteral("子涵"), QStringLiteral("一诺"), QStringLiteral("浩然"), QStringLiteral("欣怡"),
        QStringLiteral("梓萱"), QStringLiteral("宇航"), QStringLiteral("思源"), QStringLiteral("雨桐"),
        QStringLiteral("明轩"), QStringLiteral("佳琪"), QStringLiteral("俊杰"), QStringLiteral("梦瑶"),
        QStringLiteral("嘉豪"), QStringLiteral("诗涵"), QStringLiteral("博文"), QStringLiteral("若曦"),
        QStringLiteral("天佑"), QStringLiteral("语晨"), QStringLiteral("泽宇"), QStringLiteral("依琳"),
        QStringLiteral("承泽"), QStringLiteral("婉婷"), QStringLiteral("景行"), QStringLiteral("书瑶"),
        QStringLiteral("启航"), QStringLiteral("沐阳"), QStringLiteral("清妍"), QStringLiteral("星辰"),
        QStringLiteral("安然"), QStringLiteral("嘉宁"), QStringLiteral("昊天"), QStringLiteral("可欣")
    };

    for (int i = 1; i <= adminCount; ++i) {
        const QString id = (i == 1) ? QStringLiteral("admin") : paddedId(QStringLiteral("admin"), i, 2);
        const QString username = (i == 1) ? QStringLiteral("admin") : QStringLiteral("系统管理员%1").arg(i);
        User user(id, username, QStringLiteral("总部"), UserRole::Admin);
        user.setActive(true);
        dataSet.admins.push_back(user);
    }

    for (int i = 1; i <= managerCount; ++i) {
        const QString department = departments.at((i - 1) % departments.size());
        User user(paddedId(QStringLiteral("mgr"), i, 2),
                  QStringLiteral("%1经理").arg(pickName(surnames, givenNames, i + 40)),
                  department,
                  UserRole::Manager);
        user.setActive(true);
        dataSet.managers.push_back(user);
    }

    for (int i = 1; i <= salesCount; ++i) {
        const QString department = managerCount > 0
                                       ? dataSet.managers.at((i - 1) % dataSet.managers.size()).getDepartment()
                                       : departments.at((i - 1) % departments.size());
        User user(paddedId(QStringLiteral("sales"), i, 2),
                  pickName(surnames, givenNames, i),
                  department,
                  UserRole::Sales);
        user.setActive(true);
        dataSet.sales.push_back(user);
    }

    const QDateTime now = QDateTime::currentDateTime();
    for (int i = 1; i <= customerCount; ++i) {
        const QString ownerId = salesCount > 0
                                    ? dataSet.sales.at((i - 1) % dataSet.sales.size()).getUserId()
                                    : QString();
        Customer customer(
            paddedId(QStringLiteral("C"), i, 3),
            pickName(surnames, givenNames, i + 80),
            QStringLiteral("13%1").arg(800000000 + i, 9, 10, QLatin1Char('0')),
            (i % 8 == 0) ? QStringLiteral("VIP") : QStringLiteral("普通"),
            now.addDays(-(i % 45)).addSecs(i * 60),
            ownerId
        );
        dataSet.customers.push_back(customer);

        if (!ownerId.isEmpty() && i <= qMin(customerCount, 40)) {
            const User& owner = dataSet.sales.at((i - 1) % dataSet.sales.size());
            dataSet.followRecords.emplace_back(
                paddedId(QStringLiteral("FR"), i, 3),
                customer.getId(),
                owner.getUserId(),
                owner.getUsername(),
                QStringLiteral("完成客户电话回访，记录需求与后续跟进计划。"),
                now.addDays(-(i % 20)).addSecs(i * 90)
            );
        }
    }

    return dataSet;
}
