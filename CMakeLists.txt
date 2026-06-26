# CRM 客户关系管理系统构建配置
# 使用 Qt 6.5 + CMake 构建，支持 Windows/macOS 平台

cmake_minimum_required(VERSION 3.19)
project(customerControlSystem LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 依赖组件
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

# Qt 标准项目设置
qt_standard_project_setup()

# 添加可执行程序和所有源文件
qt_add_executable(customerControlSystem
    WIN32 MACOSX_BUNDLE

    # 入口文件
    SourceCode/src/main.cpp

    # UI 文件
    SourceCode/ui/logindialog.ui
    SourceCode/ui/mainwindow.ui

    # 头文件
    SourceCode/include/GlobalModels.h
    SourceCode/include/user.h
    SourceCode/include/Customer.h
    SourceCode/include/FollowRecord.h
    SourceCode/include/ICustomerRepository.h
    SourceCode/include/SQLiteCustomerRepo.h
    SourceCode/include/DatabaseManager.h
    SourceCode/include/HighSeasManager.h
    SourceCode/include/AsyncExportWorker.h
    SourceCode/include/RandomDataGenerator.h
    SourceCode/include/logindialog.h
    SourceCode/include/BaseMainWindow.h
    SourceCode/include/SalesMainWindow.h
    SourceCode/include/ManagerMainWindow.h
    SourceCode/include/AdminMainWindow.h
    SourceCode/include/customerdetaildialog.h
    SourceCode/include/followtimelinedialog.h
    SourceCode/include/userdetaildialog.h

    # 实现文件
    SourceCode/src/logindialog.cpp
    SourceCode/src/SQLiteCustomerRepo.cpp
    SourceCode/src/DatabaseManager.cpp
    SourceCode/src/HighSeasManager.cpp
    SourceCode/src/AsyncExportWorker.cpp
    SourceCode/src/RandomDataGenerator.cpp
    SourceCode/src/basemainwindow.cpp
    SourceCode/src/salesmainwindow.cpp
    SourceCode/src/managermainwindow.cpp
    SourceCode/src/adminmainwindow.cpp
    SourceCode/src/customerdetaildialog.cpp
    SourceCode/src/followtimelinedialog.cpp
    SourceCode/src/userdetaildialog.cpp
)

# 设置包含目录
target_include_directories(customerControlSystem
    PRIVATE
        SourceCode/include
        SourceCode/src
        SourceCode/ui
)

# 链接 Qt 库
target_link_libraries(customerControlSystem
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

# 安装目标
install(TARGETS customerControlSystem
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 生成 Qt 部署脚本
qt_generate_deploy_app_script(
    TARGET customerControlSystem
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
