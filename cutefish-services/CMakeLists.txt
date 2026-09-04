project(cutefish-services)
set(TARGET cutefish-services)

find_package(KF6IdleTime REQUIRED)
find_package(KWayland REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS WaylandClient)
find_package(WaylandScanner REQUIRED)

file(GLOB_RECURSE SOURCES
  "*.cpp"
  "*.h"
)

set(HEADERS "")
set(FORMS "")
set(RESOURCES "")

qt6_add_dbus_adaptor(DBUS_SOURCES
                     brightness/com.cutefish.Services.Brightness.xml
                     brightness/brightnessmanager.h BrightnessManager)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     appearance/com.cutefish.Appearance.xml
                     appearance/thememanager.h ThemeManager)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     battery/com.cutefish.Battery.xml
                     battery/battery.h Battery)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     locale/com.cutefish.Services.Language.xml
                     locale/language.h Language)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     input/mouse/com.cutefish.Services.Input.Mouse.xml
                     input/mouse/mousemanager.h Mouse)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     input/touchpad/com.cutefish.Services.Input.Touchpad.xml
                     input/touchpad/touchpadmanager.h TouchpadManager)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     power/com.cutefish.Power.xml
                     power/powermanager.h PowerManager)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
qt6_generate_wayland_protocol_client_sources(
        ${TARGET}
        PRIVATE_CODE
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/wlr-data-control-unstable-v1.xml
)
target_link_libraries(${TARGET}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::DBus
        Qt6::Xml
        KF6::IdleTime
        Qt6::WaylandClient
        Plasma::KWaylandClient
)

file(GLOB TS_FILES translations/*.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(${TARGET} translations)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${TARGET}/translations)
