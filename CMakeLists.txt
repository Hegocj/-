# 工程构建说明：
# 本文件是 Qt/CMake 工程的构建入口，负责声明项目名称、Qt 依赖组件、
# 可执行程序源码清单、链接库和安装部署脚本。源码清单中的文件共同构成
# 客户信息管理系统的模型层、数据访问层、业务服务层和界面层。
cmake_minimum_required(VERSION 3.19)
project(customerContorlSystem LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(customerContorlSystem
    WIN32 MACOSX_BUNDLE
    src/app/main.cpp


    src/ui/core/mainwindow.ui
    src/models/GlobalModels.h
    src/models/user.h
    src/models/Customer.h
    src/models/FollowRecord.h
    src/repositories/ICustomerRepository.h
    src/repositories/SQLiteCustomerRepo.h src/repositories/SQLiteCustomerRepo.cpp
    src/repositories/DatabaseManager.h src/repositories/DatabaseManager.cpp
    src/services/HighSeasManager.h src/services/HighSeasManager.cpp
    src/services/AsyncExportWorker.h src/services/AsyncExportWorker.cpp
    src/services/RandomDataGenerator.h src/services/RandomDataGenerator.cpp
    src/ui/auth/logindialog.h src/ui/auth/logindialog.cpp src/ui/auth/logindialog.ui
    src/ui/core/BaseMainWindow.h src/ui/core/basemainwindow.cpp
    src/ui/windows/SalesMainWindow.h src/ui/windows/salesmainwindow.cpp
    src/ui/windows/ManagerMainWindow.h src/ui/windows/managermainwindow.cpp
    src/ui/windows/AdminMainWindow.h src/ui/windows/adminmainwindow.cpp
    src/ui/dialogs/customerdetaildialog.h src/ui/dialogs/customerdetaildialog.cpp
    src/ui/dialogs/followtimelinedialog.h src/ui/dialogs/followtimelinedialog.cpp
    src/ui/dialogs/userdetaildialog.h src/ui/dialogs/userdetaildialog.cpp
)

target_include_directories(customerContorlSystem
    PRIVATE
        src/app
        src/models
        src/repositories
        src/services
        src/ui/auth
        src/ui/core
        src/ui/dialogs
        src/ui/windows
)

target_link_libraries(customerContorlSystem
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

install(TARGETS customerContorlSystem
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET customerContorlSystem
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
