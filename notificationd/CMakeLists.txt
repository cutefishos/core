set(SRCS main.cpp
         screenhelper.cpp
         notificationsmodel.cpp
         notificationserver.cpp
         notification.cpp
         dbus/notificationsadaptor.cpp
         resources.qrc
)

# for Ubuntu
# qt_add_dbus_adaptor(SRCS org.freedesktop.Notifications.xml notificationserver.h NotificationServer)

add_executable(cutefish-notificationd ${SRCS})

target_link_libraries(cutefish-notificationd
    Qt5::Core
    Qt5::DBus
    Qt5::Quick
    Qt5::Widgets
)

install(TARGETS cutefish-notificationd
    DESTINATION /usr/bin
    COMPONENT Runtime
)
