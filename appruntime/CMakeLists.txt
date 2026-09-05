project(cutefish-appruntime)
set(TARGET cutefish-appruntime)

set(SOURCES
    appruntime.cpp
    main.cpp
)

qt6_add_dbus_adaptor(DBUS_SOURCES
                     com.cutefish.AppRuntime.xml
                     appruntime.h AppRuntime
                     appruntimeadaptor AppRuntimeAdaptor)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
    Cutefish::Applications
    Qt6::Core
    Qt6::DBus
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
