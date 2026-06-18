cmake_minimum_required(VERSION 3.19)
project(customerContorlSystem LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(customerContorlSystem
    WIN32 MACOSX_BUNDLE
    main.cpp


    mainwindow.ui
    GlobalModels.h
    ICustomerRepository.h
    SQLiteCustomerRepo.h SQLiteCustomerRepo.cpp
    logindialog.h logindialog.cpp logindialog.ui
    basemainwindow.h basemainwindow.cpp
    salesmainwindow.h salesmainwindow.cpp
    managermainwindow.h managermainwindow.cpp
    adminmainwindow.h adminmainwindow.cpp
    customerdetaildialog.h customerdetaildialog.cpp
    followtimelinedialog.h followtimelinedialog.cpp
    user.h
    Customer.h
    FollowRecord.h
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
