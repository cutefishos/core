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

qt6_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.Session.xml
                     application.h Application
                     sessionadaptor SessionAdaptor)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Cutefish::Applications
        Qt6::Core
        Qt6::DBus
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS cutefish-kwin-wayland-session cutefish-wayland-session DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES cutefish-applications.menu DESTINATION /etc/xdg/menus/)
install(FILES cutefish-wayland.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/wayland-sessions/)
