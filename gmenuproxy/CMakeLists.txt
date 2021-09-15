find_package(AppMenuGtkModule)
find_package(KF5WindowSystem)
find_package(KF5CoreAddons)
set_package_properties(AppMenuGtkModule PROPERTIES TYPE RUNTIME)

add_definitions(-DQT_NO_CAST_TO_ASCII
-DQT_NO_CAST_FROM_ASCII
-DQT_NO_CAST_FROM_BYTEARRAY)

find_package(XCB
    REQUIRED COMPONENTS
        XCB
)

set(GMENU_DBUSMENU_PROXY_SRCS
    extend/dbusmenutypes_p.cpp
#    extend/dbusmenushortcut_p.cpp

    main.cpp
    menuproxy.cpp
    window.cpp
    menu.cpp
    actions.cpp
    dbusmenuadaptor.cpp
    gdbusmenutypes_p.cpp
    icons.cpp
    utils.cpp
 )

# qt_add_dbus_adaptor(GMENU_DBUSMENU_PROXY_SRCS ./com.canonical.dbusmenu.xml window.h Window)

add_executable(cutefish-gmenuproxy ${GMENU_DBUSMENU_PROXY_SRCS})

set_package_properties(XCB PROPERTIES TYPE REQUIRED)

target_link_libraries(cutefish-gmenuproxy
    Qt5::Core
    Qt5::X11Extras
    Qt5::DBus
    Qt5::Widgets
    KF5::CoreAddons
    KF5::WindowSystem
    XCB::XCB
)

configure_file(
    cutefish-gmenuproxy.service.in
    cutefish-gmenuproxy.service
    @ONLY
)

install(TARGETS cutefish-gmenuproxy DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cutefish-gmenuproxy.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/)
