
<img style="position:relative;margin:0px;padding:0;top:40px" src="https://i.imgur.com/cCT9KwN.png" width="104"/>
<h1 style="margin-top:0px;padding-top:0px">Louvre</h1>

<p align="left">
  <a href="https://github.com/CuarzoSoftware/Louvre/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="Louvre is released under the MIT license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/Louvre">
    <img src="https://img.shields.io/badge/version-2.7.0-brightgreen" alt="Current Louvre version." />
  </a>
</p>

Louvre is a high-performance C++ library designed for building Wayland compositors with a strong emphasis on ease of development.

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9VCmbRMl7f4ibvQqDrWpmLkXJ9W3MHHWKKE7g5oKcYSIrOut0mQEb1sDoblm9h35zUXk5zhwOwlWnM-soCtjeznhmA7yfRNqo-5a3PdwNYapM1vn4=w2400"/>

## Links

* [📖 C++ API Documentation](https://cuarzosoftware.github.io/Louvre/annotated.html)
* [🎓 Tutorial](https://cuarzosoftware.github.io/Louvre/tutorial_tmp.html)
* [🕹️ Examples](https://cuarzosoftware.github.io/Louvre/examples_page.html)
* [📦 Downloads](https://cuarzosoftware.github.io/Louvre/downloads_page.html)
* [⚙️ Environment](https://cuarzosoftware.github.io/Louvre/environment_page.html)
* [💬 Contact](https://cuarzosoftware.github.io/Louvre/contact_page.html)
* [🌟 Projects Using Louvre](https://github.com/CuarzoSoftware/Louvre/blob/gallery/README.md)
* [🎞️ Watch Video Demo](https://youtu.be/k-DuNyF1XDg?si=qvxwRTe_OIUMDudi)

Creating a Wayland compositor can be a daunting undertaking, often spanning months or even years of dedication. This challenging task involves mastering Linux input and graphic APIs, managing graphic buffers, and meticulously implementing numerous Wayland protocols and their respective interfaces.

Fortunately, Louvre simplifies this intricate process by handling all the complex low-level tasks on your behalf. It even provides a default way for managing protocols, enabling you to have a basic but functional compositor from day one and progressively explore and customize its functionality to precisely match your requirements.

## ⭐ Features

* Fractional Scaling (with optional oversampling)
* Direct Scanout (primary plane)
* VSync Control
* Gamma Correction
* Screencasting (compatible with PipeWire via xdg-desktop-portal-wlr)
* Painter API and Scene System
* Multi-GPU Support
* Multi-Session Support
* Double and Triple Buffering
* Persistent Clipboard
* Rootful XWayland (rootless mode is not yet supported, however, interesting projects such as [Wayland Transpositor](https://github.com/wayland-transpositor/wprs), [Wayland Proxy Virtwl](https://github.com/talex5/wayland-proxy-virtwl), and [Xwayland Satellite](https://github.com/Supreeeme/xwayland-satellite) can provide a rootless experience).

## 🧩 Protocols

* Wayland
* XDG Activation
* XDG Shell
* XDG Decoration
* XDG Output
* Presentation Time
* Linux DMA-Buf
* Single Pixel Buffer
* Viewporter
* Fractional Scale
* Tearing Control
* Session Lock
* Idle Notify
* Idle Inhibit
* Content Type Hint
* Wlr Gamma Control
* Wlr Layer Shell
* Wlr Screen Copy
* Wlr Foreign Toplevel Management
* Foreign Toplevel List
* Pointer Gestures
* Pointer Constraints
* Relative Pointer

## 🖌️ Rendering

Within Louvre, you have the flexibility to either employ your own OpenGL ES 2.0 shaders/programs, use the LPainter class for fundamental 2D rendering, or leverage the LScene and LView system, which manages buffer damage and can also handle input events for you. Additionally, it's possible to combine these three approaches as needed.

## 🔲 Tested Hardware

* Intel (i915 driver)
* AMD (amdgpu driver)
* Nvidia (proprietary and nouveau drivers)
* Mali (lima driver)

## 💬 Client-Compositor Buffer Sharing

* Shared Memory
* Wayland EGL
* Linux DMA-Buf

## 💻 Graphic Backends

* DRM/KMS
* Wayland

## 🕹️ Input Backends

* Libinput
* Wayland

## ⏲️ Performance

Louvre offers excellent performance. A benchmark consisting of rendering numerous moving [wl_subsurfaces](https://wayland.app/protocols/wayland#wl_subsurface) (opaque and translucent), in which the [louvre-weston-clone](https://cuarzosoftware.github.io/Louvre/md_md__examples.html#weston) example compositor was tested, shows that Louvre can maintain a high FPS rate even in complex scenarios. Furthermore, it uses fewer CPU and GPU resources than popular compositors like Weston and Sway.

#### Benchmark Environment

<table>
  <tr>
    <td><strong>Machine</strong></td>
    <td>MacBook Pro A1398 (Retina, 15-inch, Mid 2015)</td>
  </tr>
  <tr>
    <td><strong>CPU</strong></td>
    <td>Intel Core i7-4770HQ @ 2.20GHz (up to 3.4GHz) with 6MB shared L3 cache</td>
  </tr>
  <tr>
    <td><strong>Memory</strong></td>
    <td>16GB of 1600MHz DDR3L</td>
  </tr>
  <tr>
    <td><strong>GPU</strong></td>
    <td>Intel Iris Pro Graphics - i915 (Intel Graphics) version 1.6.0 (20201103)</td>
  </tr>
  <tr>
    <td><strong>Display</strong></td>
    <td>15-inch Retina Display with single mode 2880x1800@60Hz</td>
  </tr>
  <tr>
    <td><strong>OS</strong></td>
    <td>Linux Mint 21 - Linux 5.15.0-86-generic</td>
  </tr>
</table>

> If you're interested in the details of how the benchmark operates and would like to try it yourself, please refer to [this link](https://github.com/CuarzoSoftware/Louvre/tree/main/src/benchmark).

Here is a graph illustrating the benchmark results. It displays the average FPS of each compositor rendering 1 to 50 moving surfaces using double buffering on a HiDPI display.

> The benchmark results have not been updated since 2023 and may not accurately reflect the current performance of the tested compositors.

### FPS

<img src="https://lh3.googleusercontent.com/pw/AIL4fc_fcGPw-Yh1zkqxKdfEQucQVXH853Py1YXtTk7jHVACzIaYmYCId07D0hsdJ-FArkERPjJQR2shCc4swA7b1cy9X9EhvFPqLOR_kxV-C1eVQHey2m8=w2400"/>

Most Wayland compositors use a single thread, which drastically slows down their performance when rendering complex scenarios. The reason for this is due to **vertical sync**, where the compositor must wait a few milliseconds before it can swap the framebuffer it just rendered with the one being displayed on screen. This is done in order to synchronize the swapping with the display refresh rate (*vblank*) and avoid the **tearing effect**. When working with a single thread, compositors have "dead times" that prevent them from processing and rendering the content in time for the next frame. That's why they end up skipping a *vblank*, causing their refresh rate to drop in half (or more).
To avoid this problem, Louvre works with multiple threads. Each output (display) renders its content on its own thread, allowing the compositor to continue processing requests and render to other outputs while one is waiting for a vblank. This prevent Louvre compositors from having "dead times" and therefore allows them to maintain a high refresh rate.

### CPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9YhNEf4Rjsqsz49aFtMjyjifDxE9aKgxoOLsfTdJwIu-CqEJr3MJHALX9pgJp05kYJY1z1YBTZjUCQcIAf-gjvRAsumqzEyDm88t1E9SL4aCzaZBo=w2400"/>

The graph on the left displays the raw CPU consumption results, which might suggest that Louvre uses more CPU resources. However, this comparison isn't entirely fair, as Louvre's refresh rate is nearly double that of the others (60 FPS vs 30 FPS avg). When we divide the CPU consumption by the frames per second (FPS), as shown in the graph on the right, it becomes evident that Louvre, in fact, uses fewer CPU resources relative to FPS compared to the other compositors.

### GPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc-bzBT_dchcsaVgIOE1iw4iQ2KF_AZ9WItQFXSf2bILxNiaQSpLaXaEkR5p06jb7qdjOqZeYV2m-vHt1KyBed7TH2IQ0jas-lkmxIbcFRAj1w0BojU=w2400"/>

Similarly as with CPU consumption, we can observe that Louvre uses fewer GPU resources relative to FPS than the other compositors.

## 🔨 Upcoming Features

* Wlr Output Management
* Cursor Shape Protocol
* DRM Overlay Planes Control
* DRM Synchronization Object
* DRM Lease Protocol
* Rootless XWayland
* Tablet Events Protocol
* Virtual Keyboard Protocol
* Input Methods Protocol