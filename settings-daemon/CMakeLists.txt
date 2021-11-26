project(cutefish-settings-daemon)
set(TARGET cutefish-settings-daemon)

find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)

pkg_check_modules(XORGLIBINPUT xorg-libinput IMPORTED_TARGET)
pkg_check_modules(XORGSERVER xorg-server IMPORTED_TARGET)
pkg_check_modules(SYNAPTICS xorg-synaptics IMPORTED_TARGET GLOBAL)

file (GLOB_RECURSE SRCS
  "*.cpp"
  "*.h"
  "theme/*.h"
  "theme/*.cpp"
  "brightness/*.h"
  "brightness/*.cpp"
  "brightness/*.c"
  "battery/*.cpp"
  "battery/*.h"
  "language/*.cpp"
  "language/*.h"
  "dock/*.cpp"
  "dock/*.h"
  "mouse/*.h"
  "mouse/*.cpp"
  "touchpad/*.h"
  "touchpad/*.cpp"
  "touchpad/x11/*.h"
  "touchpad/x11/*.cpp"
)

set(SOURCES ${SRCS})
set(HEADERS "")
set(FORMS "")
set(RESOURCES "")

qt5_add_dbus_adaptor(DBUS_SOURCES
                     brightness/com.cutefish.Brightness.xml
                     brightness/brightnessmanager.h BrightnessManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     theme/com.cutefish.Theme.xml
                     theme/thememanager.h ThemeManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     battery/com.cutefish.PrimaryBattery.xml
                     battery/battery.h Battery)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     language/com.cutefish.Language.xml
                     language/language.h Language)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     dock/com.cutefish.Dock.xml
                     dock/dock.h Dock)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     mouse/com.cutefish.Mouse.xml
                     mouse/mousemanager.h Mouse)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     touchpad/com.cutefish.Touchpad.xml
                     touchpad/touchpadmanager.h TouchpadManager)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        Qt5::Xml
        ${X11_LIBRARIES}
        X11::X11
        X11::Xi
        X11::XCB
        ${XCB_LIBRARIES}
        PkgConfig::XORGLIBINPUT
        PkgConfig::XORGSERVER
        PkgConfig::SYNAPTICS
)

file(GLOB TS_FILES translations/*.ts)
qt5_create_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(${TARGET} translations)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${TARGET}/translations)
