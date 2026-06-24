/**
 * @file RandomDataGenerator.h
 * @brief 本地演示数据随机生成器声明。
 *
 * 研究定位：本文件属于服务层，负责按指定数量生成管理员、经理、销售、客户和跟进记录
 * 等演示数据。生成器不直接访问数据库，只返回模型对象，具体持久化由仓库层完成。
 *
 * 主要职责：根据传入数量构造中文姓名、11 位手机号、部门归属、客户负责人和跟进记录，
 * 为 SQLiteCustomerRepo 的初始化过程提供可配置的数据来源。
 */
#ifndef RANDOMDATAGENERATOR_H
#define RANDOMDATAGENERATOR_H

#include "GlobalModels.h"

#include <vector>

struct RandomDataConfig
{
    int customerCount = 120;
    int salesCount = 20;
    int managerCount = 5;
    int adminCount = 1;
};

struct RandomDataSet
{
    std::vector<User> admins;
    std::vector<User> managers;
    std::vector<User> sales;
    std::vector<Customer> customers;
    std::vector<FollowRecord> followRecords;
};

class RandomDataGenerator
{
public:
    static RandomDataSet generate(const RandomDataConfig& config);
};

#endif // RANDOMDATAGENERATOR_H
