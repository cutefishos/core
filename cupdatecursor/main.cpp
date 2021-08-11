#include <QGuiApplication>
#include <QX11Info>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

#include <X11/X.h>
#include <X11/Xcursor/Xcursor.h>

inline void applyTheme(const QString &theme, int size)
{
    Display *display = QX11Info::display();

    if (!theme.isEmpty())
        XcursorSetTheme(display, QFile::encodeName(theme));

    if (size > 0)
        XcursorSetDefaultSize(display, size);

    Cursor handle = XcursorLibraryLoadCursor(display, "left_ptr");
    XDefineCursor(display, DefaultRootWindow(display), handle);
    XFreeCursor(display, handle);
    XFlush(display);

    // For KWin
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kcminputrc",
                       QSettings::IniFormat);
    settings.beginGroup("Mouse");
    settings.setValue("cursorTheme", theme);
    settings.setValue("cursorSize", size);
    settings.endGroup();
    settings.sync();
}

int main(int argc, char *argv[])
{
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication a(argc, argv);

    if (argc != 3)
        return 1;

    if (!QX11Info::isPlatformX11())
        return 2;

    QString theme = QFile::decodeName(argv[1]);
    QString size = QFile::decodeName(argv[2]);

    applyTheme(theme, size.toInt());

    return 0;
}
