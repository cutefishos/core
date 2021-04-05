# Components

System backend and start session and more.

## Compile dependencies

```shell
sudo pacman -S extra-cmake-modules pkgconf qt5-base qt5-quickcontrols2 qt5-x11extras qt5-tools kwindowsystem polkit polkit-qt5 pulseaudio
```

For Ubuntu:
```shell
sudo apt install libpolkit-agent-1-dev libpolkit-qt5-1-dev libpulse-dev libsm-dev libxtst-dev\
    libxcb-randr0-dev libxcb-shape0-dev libxcb-xfixes0-dev libxcb-composite0-dev libxcb-damage0-dev
```
(Yes it's annoying that so many xcb's packages here is needed to install. Isn't there a way to install one package and these `libxcb`s all get ready?)

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
