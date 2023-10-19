# 📦 Downloads

Louvre relies on the following libraries:

* **Wayland Server** >= 1.16
* **EGL** >= 1.5.0
* **GLES 2.0** >= 13.0.6
* **DRM** >= 2.4.85
* **SRM** >= 0.3.0
* **GBM** >= 22.2.0
* **Evdev** >= 1.5.6
* **Libinput** >= 1.6.3
* **XCursor** >= 1.1.15
* **XKB Common** >= 0.7.1
* **Pixman** >= 0.40.0
* **Libseat** >= 0.6.4
* **FreeImage** >= 3.18.0

The examples also require:

* **Libicu** >= 72.1
* **FontConfig** >= 2.14.1
* **FreeType** >= 2.8.1

And can easily be built with [Meson](https://mesonbuild.com/).

## Debian (Ubuntu, Mint, etc)

If your distribution is based on Debian, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo apt update
$ sudo apt upgrade
$ sudo apt install build-essential meson libegl-dev mesa-common-dev libgles2-mesa-dev libdrm-dev libgbm-dev libevdev-dev libinput-dev libxcursor-dev libxkbcommon-dev libpixman-1-dev libwayland-dev hwinfo libseat-dev libfreeimage-dev libicu-dev libfontconfig-dev libfreetype-dev
```

If the [hwinfo](https://github.com/vcrhonek/hwdata) or [libdisplay-info](https://gitlab.freedesktop.org/emersion/libdisplay-info) packages are not available in your distribution, please download and install them manually in the specified order:

1. [hwinfo](https://packages.ubuntu.com/focal/hwdata)
2. [libdisplay-info0](https://packages.ubuntu.com/lunar/libdisplay-info0)
3. [libdisplay-info-dev](https://packages.ubuntu.com/lunar/libdisplay-info-dev)

To install SRM, follow the instructions provided [here](https://cuarzosoftware.github.io/SRM/md_md__downloads.html).

It is also recommended to install [weston-terminal](https://gitlab.freedesktop.org/wayland/weston), which is compatible with Wayland and is used throughout the tutorial and the examples.

```
$ sudo apt install weston
```

## Red Hat (Fedora, CentOS, etc)

If your distribution is based on Red Hat, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo dnf update
$ sudo dnf install @development-tools make automake gcc gcc-c++ meson libwayland-server wayland-devel libinput-devel libevdev-devel libudev-devel mesa-libEGL-devel libxkbcommon-devel libXcursor-devel pixman-devel libdrm-devel libgbm-devel libseat-devel libfreeimage-devel fontconfig-devel freetype-devel libicu-devel
```

To install SRM, follow the instructions provided [here](https://cuarzosoftware.github.io/SRM/md_md__downloads.html).

It is also recommended to install [weston-terminal](https://gitlab.freedesktop.org/wayland/weston) which is compatible with Wayland and is used throughout the tutorial and the examples.

```
$ sudo dnf install weston
```

# Building Louvre

Run the following commands to compile and install Louvre:

```
$ git clone https://github.com/CuarzoSoftware/Louvre.git
$ cd Louvre/src
$ meson setup build -Dbuildtype=custom
$ cd build
$ sudo meson install
```

This will install the library in:

```
/usr/lib
```

The headers in:

```
/usr/include/Louvre
```

The backends and assets in:

```
/usr/etc/Louvre/
```

And examples in:

```
/usr/bin
```

To ensure that everything is functioning correctly, you can test one of the available [examples](md_md__examples.html).