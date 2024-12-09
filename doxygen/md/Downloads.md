# 📦 Downloads {#downloads_page}

## Pre-built Binaries

Pre-built binaries are provided for the following distributions. Please be aware that their versions may not always match the latest Louvre release.

* **Arch** : [louvre](https://aur.archlinux.org/packages/louvre) - *Thanks to [@TrialnError](https://aur.archlinux.org/account/TrialnError)*.
* **Fedora** : [cuarzo-louvre](https://copr.fedorainfracloud.org/coprs/cuarzo/software/) - *By [Cuarzo Software](https://github.com/CuarzoSoftware) (always up to date)*.
* **NixOS** : [louvre](https://search.nixos.org/packages?channel=unstable&show=louvre&from=0&size=50&sort=relevance&type=packages&query=louvre) - *Thanks to [Marco Rebhan](https://github.com/2xsaiko)*.

## Manual Building

Louvre relies on the following libraries:

* **wayland-server** >= 1.20.0
* **gl** >= 1.2
* **egl** >= 1.5
* **glesv2** >= 3.2
* **libdrm** >= 2.4.113
* **srm** >= 0.11.0
* **libudev** >= 249
* **libinput** >= 1.20.0
* **xcursor** >= 1.2.0
* **xkbcommon** >= 1.4.0
* **pixman-1** >= 0.40.0
* **libseat** >= 0.6.4

Wayland backends require:

* **wayland-client** >= 1.20.0
* **wayland-egl** >= 18.1.0

And the examples require:

* **iccu** >= 72.1
* **fontconfig** >= 2.13.1
* **freetype2** >= 24.1.18

And can easily be built with [Meson](https://mesonbuild.com/).

## Debian (Ubuntu, Mint, etc)

If your distribution is based on Debian, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo apt update
$ sudo apt upgrade
$ sudo apt install build-essential meson libegl-dev mesa-common-dev libgles2-mesa-dev libdrm-dev libevdev-dev libinput-dev libxcursor-dev libxkbcommon-dev libpixman-1-dev libwayland-dev hwinfo libseat-dev libicu-dev libfontconfig-dev libfreetype-dev
```

To install SRM, follow the instructions provided [here](https://cuarzosoftware.github.io/SRM/md_md__downloads.html).

It is also recommended to install [weston-terminal](https://gitlab.freedesktop.org/wayland/weston), which is compatible with Wayland and is used throughout the tutorial and the examples.

```
$ sudo apt install weston
```

## Red Hat (Fedora, CentOS, etc)

If your distribution is based on Red Hat, all tools and almost all dependencies can be installed with the following commands:

```
$ sudo dnf update
$ sudo dnf install @development-tools make automake gcc gcc-c++ meson libwayland-client libwayland-server wayland-devel libinput-devel libevdev-devel libudev-devel mesa-libEGL-devel libxkbcommon-devel libXcursor-devel pixman-devel libdrm-devel libseat-devel fontconfig-devel freetype-devel libicu-devel
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
$ meson setup build
$ cd build
$ meson install
$ sudo ldconfig
```

To ensure that everything is functioning correctly, you can test one of the available [examples](md_md__examples.html).