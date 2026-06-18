cmake_minimum_required(VERSION 3.19)
project(customerContorlSystem LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(customerContorlSystem
    WIN32 MACOSX_BUNDLE
    main.cpp


    mainwindow.ui
    GlobalModels.h
    ICustomerRepository.h
    MockCustomerRepo.h
    logindialog.h logindialog.cpp logindialog.ui
    basemainwindow.h basemainwindow.cpp
    salesmainwindow.h salesmainwindow.cpp
    managermainwindow.h managermainwindow.cpp
    adminmainwindow.h adminmainwindow.cpp
    customercarddialog.h customercarddialog.cpp
    followtimelinedialog.h followtimelinedialog.cpp
)

target_link_libraries(customerContorlSystem
    PRIVATE
        Qt::Core
        Qt::Widgets
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
