# System

System backend and start session and more.

## Compile dependencies

```shell
sudo pacman -S extra-cmake-modules pkgconf qt5-base qt5-quickcontrols2 qt5-x11extras qt5-tools kwindowsystem polkit polkit-qt5 pulseaudio polkit-qt5
```

## Runtime

```shell
pulseaudio
```

## Build

```shell
mkdir build
cd build
cmake ..
make
```

## Install

```shell
sudo make install
```

## License

This project has been licensed by GPLv3.