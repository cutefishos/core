project(cutefish-powerman)
set(TARGET cutefish-powerman)

set(SOURCES
    main.cpp
    application.cpp
    lidwatcher.cpp
    cpu/cpuitem.cpp
    cpu/cpumanagement.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCES
                     cpu/org.cutefish.CPUManagement.xml
                     cpu/cpumanagement.h CPUManagement)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Quick
        Qt5::DBus
        Qt5::X11Extras
)

install(TARGETS ${TARGET} DESTINATION /usr/bin)
