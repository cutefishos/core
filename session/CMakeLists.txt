project(cutefish-session)
set(TARGET cutefish-session)

set(SOURCES
    application.cpp
    main.cpp
    process.cpp
    processmanager.cpp
    networkproxymanager.cpp

    powermanager/power.cpp
    powermanager/powerproviders.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.Session.xml
                     application.h Application
                     sessionadaptor SessionAdaptor)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

find_package(KF5WindowSystem)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Quick
        Qt5::DBus
        Qt5::X11Extras
        KF5::WindowSystem
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES cutefish-xsession.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/xsessions/)
