project(cutefish-powerman)
set(TARGET cutefish-powerman)

find_package(KF5IdleTime)
find_package(X11)
find_package(XCB
    REQUIRED COMPONENTS
        XCB
        DPMS
)

set(XCB_LIBS
    XCB::XCB
    XCB::DPMS
)

set(SOURCES
    main.cpp
    application.cpp
    lidwatcher.cpp
    action.cpp
    idlemanager.cpp
    dimdisplayaction.cpp
    cpu/cpuitem.cpp
    cpu/cpumanagement.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCES
                     cpu/com.cutefish.CPUManagement.xml
                     cpu/cpumanagement.h CPUManagement)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.PowerManager.xml
                     application.h Application)
qt_add_dbus_interface(DBUS_SOURCES org.freedesktop.ScreenSaver.xml screenlocker_interface)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Quick
        Qt5::DBus
        Qt5::X11Extras

        KF5::IdleTime
        ${XCB_LIBS}
        ${X11_LIBRARIES}
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
