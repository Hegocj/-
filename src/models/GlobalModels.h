/**
 * @file GlobalModels.h
 * @brief 全局业务模型聚合头文件。
 *
 * 研究定位：本文件属于模型层的聚合入口，用于把 User、Customer、FollowRecord
 * 三个核心实体集中暴露给仓库层和界面层。它减少调用方重复包含多个模型头文件的成本。
 *
 * 主要职责：统一转发系统最常用的业务模型定义，不承载业务逻辑。
 */
#ifndef GLOBALMODELS_H
#define GLOBALMODELS_H

// 🌟 这里只负责转发，把拆分出去的独立实体重新聚拢
#include "user.h"
#include "Customer.h"
#include "FollowRecord.h"

#endif // GLOBALMODELS_H
