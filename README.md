# Core

System backend and start session and more.

## Compile dependencies

Install Qt 6, KDE Frameworks 6, NetworkManager, Polkit, Fontconfig, FreeType
and the development tools provided by your distribution. The currently
retained settings daemon has its remaining input-backend dependencies
declared in `settings-daemon/CMakeLists.txt`.

## Runtime

For the Wayland session, install KWin Wayland and select `Cutefish (KWin
Wayland)` from the display manager. The session entry launches KWin with
Wayland enabled and starts `cutefish-session` inside the
compositor. The session starts the retained settings daemon, which in turn
starts the desktop components after its D-Bus service is ready.

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
```

## Install

```shell
sudo make install
```

## License

This project has been licensed by GPLv3.
