# Installation

Louvre depends on the following libraries:

* **Wayland Server** >= 1.16
* **EGL** >= 1.5.0
* **GLES 2.0** >= 13.0.6
* **DRM** >= 2.4.85
* **SRM** >= 0.1.0
* **GBM** >= 22.2.0
* **Evdev** >= 1.5.6
* **Libinput** >= 1.6.3
* **XCursor** >= 1.1.15
* **XKB Common** >= 0.7.1
* **Pixman** >= 0.40.0
* **Libseat** >= 0.6.4
* **FreeImage** >= 3.18.0

Examples only:

* **FontConfig** >= 2.14.1
* **FreeType** >= 2.8.1
* **Glib 2.0** >= 2.58.3


And can easily be built with [Meson](https://mesonbuild.com/).

## Debian (Ubuntu, Mint, etc)

If your distribution is based on Debian, almost all dependencies can be installed with the following commands:

```
$ sudo apt update
$ sudo apt upgrade
$ sudo apt install build-essential libegl-dev mesa-common-dev libgles2-mesa-dev libdrm-dev libgbm-dev libevdev-dev libinput-dev libxcursor-dev libxkbcommon-dev libpixman-1-dev libwayland-dev libseat-dev libfreeimage-dev libfontconfig-dev libfreetype-dev libglib2.0-dev
```

And Meson with:

```
$ sudo apt install meson
```

To install SRM, follow the instructions [here](https://github.com/CuarzoSoftware/SRM).

It is also recommended to install **weston-terminal**, which is compatible with Wayland and will be used throughout the tutorial.

```
$ sudo apt install weston
```

## Red Hat (Fedora, CentOS, etc)

If your distribution is based on Red Hat, almost all dependencies can be installed with the following commands:

```
$ sudo dnf update
$ sudo dnf install @development-tools make automake gcc gcc-c++ kernel-devel libwayland-server wayland-devel libinput-devel libevdev-devel libudev-devel mesa-libEGL-devel libxkbcommon-devel libXcursor-devel pixman-devel libdrm-devel libgbm-devel libseat-devel libfreeimage-devel fontconfig-devel freetype-devel libglib2.0-devel
```
And Meson with:

```
$ sudo dnf install meson
```

To install SRM, follow the instructions [here](https://github.com/CuarzoSoftware/SRM).

It is also recommended to install **weston-terminal**, which is compatible with Wayland and will be used throughout the tutorial.

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

# Examples {#Examples}

Let's run the example compositors included in the library to verify that everything is working correctly.\n
Switch to a free TTY session by pressing ```CTRL+ALT+(F1,F2,...,FN)``` or by running the ```$ sudo chvt N``` command, where N is the number of the TTY.

> ⚠️ Before running, keep in mind that you can close the compositor by pressing **Ctrl+Shift+Esc**. Additionally **weston-terminal** can be launched by pressing **F1** or **fn+F1** depending on your keyboard configuration.

If your current desktop environment is based on X11, simply run one of the following examples:

#### MacOS X lookalike example:

```
$ louvre-views
```

#### Weston lookalike example:

```
$ louvre-weston-clone
```

#### Basic default compositor:

```
$ louvre-default
```

If your current desktop environment is based on Wayland, prepend the following environment variables to avoid conflicts:

```
$ WAYLAND_DISPLAY=wayland-1 LOUVRE_WAYLAND_DISPLAY=wayland-1 louvre-views
```

# Debugging

The **LOUVRE_DEBUG** environment variable can be set to an integer in the range [0,4] to display debugging information from the library with different levels of verbosity.\n
Check Louvre::LLog documentation for more information.