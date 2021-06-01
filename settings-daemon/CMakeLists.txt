project(cutefish-settings-daemon)
set(TARGET cutefish-settings-daemon)

find_package(PulseAudio)

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
)

set(SOURCES ${SRCS})
set(HEADERS "")
set(FORMS "")
set(RESOURCES "")

qt5_add_dbus_adaptor(DBUS_SOURCES
                     audio/org.cutefish.Audio.xml
                     audio/audiomanager.h AudioManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     brightness/org.cutefish.Brightness.xml
                     brightness/brightnessmanager.h BrightnessManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     theme/org.cutefish.Theme.xml
                     theme/thememanager.h ThemeManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     battery/org.cutefish.PrimaryBattery.xml
                     battery/battery.h Battery)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     language/org.cutefish.Language.xml
                     language/language.h Language)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     dock/org.cutefish.Dock.xml
                     dock/dock.h Dock)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        ${PULSEAUDIO_LIBRARY}
)

file(GLOB TS_FILES translations/*.ts)
qt5_create_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(${TARGET} translations)

install(TARGETS ${TARGET} DESTINATION /usr/bin)
install(FILES ${QM_FILES} DESTINATION /usr/share/${TARGET}/translations)