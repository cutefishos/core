project(cutefish-settings-daemon)
set(TARGET cutefish-settings-daemon)

find_package(PulseAudio)
find_package(X11 REQUIRED)
pkg_check_modules(XORGLIBINPUT xorg-libinput IMPORTED_TARGET)

file (GLOB_RECURSE SRCS
  "*.cpp"
  "*.h"
  "audio/*.cpp"
  "audio/*.h"
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
)

set(SOURCES ${SRCS})
set(HEADERS "")
set(FORMS "")
set(RESOURCES "")

qt5_add_dbus_adaptor(DBUS_SOURCES
                     audio/com.cutefish.Audio.xml
                     audio/audiomanager.h AudioManager)
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
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        ${PULSEAUDIO_LIBRARY}
        ${X11_LIBRARIES}
        X11::X11
        X11::Xi
        PkgConfig::XORGLIBINPUT
)

file(GLOB TS_FILES translations/*.ts)
qt5_create_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(${TARGET} translations)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${TARGET}/translations)
