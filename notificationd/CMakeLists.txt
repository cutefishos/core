set(SRCS main.cpp
         application.cpp
         datehelper.cpp
         screenhelper.cpp
         notificationsmodel.cpp
         notificationserver.cpp
         notification.cpp
         notificationpopup.cpp
         notificationwindow.cpp
         historymodel.cpp
         settings.cpp
         utils.cpp
         dbus/notificationsadaptor.cpp
         resources.qrc
)

# for Ubuntu
# qt_add_dbus_adaptor(SRCS org.freedesktop.Notifications.xml notificationserver.h NotificationServer)

qt6_add_dbus_adaptor(DBUS_SOURCES com.cutefish.Notification.xml application.h Application)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

find_package(KF6WindowSystem)

add_executable(cutefish-notificationd ${SRCS} ${DBUS_SOURCES})

target_link_libraries(cutefish-notificationd
    Qt6::Core
    Qt6::DBus
    Qt6::Quick
    Qt6::Widgets
    KF6::WindowSystem
)

install(TARGETS cutefish-notificationd
    DESTINATION /usr/bin
    COMPONENT Runtime
)

file(GLOB TS_FILES translations/*.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(notificationd_translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(cutefish-notificationd notificationd_translations)

install(FILES ${QM_FILES} DESTINATION /usr/share/cutefish-notificationd/translations)
