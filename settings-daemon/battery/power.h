#ifndef POWER_H
#define POWER_H

/* UPower */
#define UP_DBUS_SERVICE           "org.freedesktop.UPower"
#define UP_DBUS_PATH              "/org/freedesktop/UPower"
#define UP_DBUS_INTERFACE         "org.freedesktop.UPower"
#define UP_DBUS_INTERFACE_DEVICE  UP_DBUS_INTERFACE ".Device"
#define UP_UDI_PREFIX             "/org/freedesktop/UPower"

typedef enum {
    UP_DEVICE_KIND_UNKNOWN,
    UP_DEVICE_KIND_LINE_POWER,
    UP_DEVICE_KIND_BATTERY,
    UP_DEVICE_KIND_UPS,
    UP_DEVICE_KIND_MONITOR,
    UP_DEVICE_KIND_MOUSE,
    UP_DEVICE_KIND_KEYBOARD,
    UP_DEVICE_KIND_PDA,
    UP_DEVICE_KIND_PHONE,
    UP_DEVICE_KIND_MEDIA_PLAYER,
    UP_DEVICE_KIND_TABLET,
    UP_DEVICE_KIND_COMPUTER,
    UP_DEVICE_KIND_GAMING_INPUT,
    UP_DEVICE_KIND_LAST
} UpDeviceKind;

#endif // POWER_H
