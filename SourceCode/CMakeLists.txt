# CRM 客户关系管理系统构建配置
# 使用 Qt 6.5 + CMake 构建，支持 Windows/macOS 平台

cmake_minimum_required(VERSION 3.19)
project(customerControlSystem LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 UI 文件搜索路径（相对于源文件目录）
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 查找 Qt 依赖组件
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

# Qt 标准项目设置
qt_standard_project_setup()

# 定义源文件列表
set(HEADERS
    include/GlobalModels.h
    include/user.h
    include/Customer.h
    include/FollowRecord.h
    include/ICustomerRepository.h
    include/SQLiteCustomerRepo.h
    include/DatabaseManager.h
    include/HighSeasManager.h
    include/AsyncExportWorker.h
    include/RandomDataGenerator.h
    include/logindialog.h
    include/BaseMainWindow.h
    include/SalesMainWindow.h
    include/ManagerMainWindow.h
    include/AdminMainWindow.h
    include/customerdetaildialog.h
    include/followtimelinedialog.h
    include/userdetaildialog.h
)

set(SOURCES
    src/main.cpp
    src/logindialog.cpp
    src/SQLiteCustomerRepo.cpp
    src/DatabaseManager.cpp
    src/HighSeasManager.cpp
    src/AsyncExportWorker.cpp
    src/RandomDataGenerator.cpp
    src/basemainwindow.cpp
    src/salesmainwindow.cpp
    src/managermainwindow.cpp
    src/adminmainwindow.cpp
    src/customerdetaildialog.cpp
    src/followtimelinedialog.cpp
    src/userdetaildialog.cpp
)

set(UIS
    src/logindialog.ui
    src/mainwindow.ui
)

# 添加可执行程序
qt_add_executable(customerControlSystem
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    ${UIS}
)

# 设置包含目录
target_include_directories(customerControlSystem
    PRIVATE
        include
        src
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
