<img style="position:relative;margin:0px;padding:0;top:40px" src="https://i.imgur.com/cCT9KwN.png" width="104"/>
<h1 style="margin-top:0px;padding-top:0px">Louvre</h1>

<p align="left">
  <a href="https://github.com/CuarzoSoftware/Louvre/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-LGPLv2.1-blue.svg" alt="Louvre is released under the LGPLv2.1 license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/Louvre">
    <img src="https://img.shields.io/badge/version-3.0.0-brightgreen" alt="Current Louvre version." />
  </a>
</p>

Louvre is a C++ library for building Wayland compositors. It uses the Factory Method pattern to provide default implementations for all major functionality and protocols, which you can gradually override.
This approach lets you see results from day one, potentially easing the learning curve while still giving you full control to create a unique, custom compositor.

> Check the releases section for stable versions. The web documentation corresponds to the latest release.

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9VCmbRMl7f4ibvQqDrWpmLkXJ9W3MHHWKKE7g5oKcYSIrOut0mQEb1sDoblm9h35zUXk5zhwOwlWnM-soCtjeznhmA7yfRNqo-5a3PdwNYapM1vn4=w2400"/>

## Links

- [📖 C++ API Documentation](https://cuarzosoftware.github.io/Louvre/annotated.html)
- [🎓 Tutorial](https://cuarzosoftware.github.io/Louvre/tutorial_tmp.html)
- [🕹️ Examples](https://cuarzosoftware.github.io/Louvre/examples_page.html)
- [📦 Downloads](https://cuarzosoftware.github.io/Louvre/downloads_page.html)
- [⚙️ Environment](https://cuarzosoftware.github.io/Louvre/environment_page.html)
- [💬 Contact](https://cuarzosoftware.github.io/Louvre/contact_page.html)
- [🌟 Projects Using Louvre](https://github.com/CuarzoSoftware/Louvre/blob/gallery/README.md)
- [🎞️ Watch Video Demo](https://youtu.be/k-DuNyF1XDg?si=qvxwRTe_OIUMDudi)

## ⭐ Features

- Explicit Synchronization
- Fractional Scaling (with optional antialias)
- Direct Scanout (WIP)
- VSync Control
- Gamma Correction
- Screencasting (WIP)
- Hybrid-GPUs Support
- Multi-Session Support
- Double and Triple Buffering
- Persistent Clipboard
- Rootful XWayland (rootless mode is not supported, however, interesting projects such as [Wayland Transpositor](https://github.com/wayland-transpositor/wprs), [Wayland Proxy Virtwl](https://github.com/talex5/wayland-proxy-virtwl), and [Xwayland Satellite](https://github.com/Supreeeme/xwayland-satellite) can provide a rootless experience).

## 🧩 Protocols

- Content Type Hint
- Cursor Shape
- DRM Lease
- DRM synchronization object
- Foreign Toplevel List
- Fractional Scale
- Idle Inhibit
- Idle Notify
- Image Capture Source
- Mesa Wayland DRM
- Linux DMA-Buf
- Lvr Background Blur
- Lvr Invisible Region
- Lvr SVG Path
- Pointer Constraints
- Pointer Gestures
- Presentation Time
- Relative Pointer
- Session Lock
- Single Pixel Buffer
- Tearing Control
- Viewporter
- Wayland
- Wlr Foreign Toplevel Management
- Wlr Gamma Control
- Wlr Layer Shell
- Wlr Output Management
- XDG Activation
- XDG Decoration
- XDG Output
- XDG Shell

## 🕹 Backends

- DRM/KMS
- Wayland
- Offscreen

## 💻 Graphics APIs

- OpenGL
- Raster
- Vulkan (WIP)
