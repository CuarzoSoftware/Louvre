# 🏠 Home

Louvre is a high-performance C++ library designed for building Wayland compositors with a strong emphasis on ease of development.

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9VCmbRMl7f4ibvQqDrWpmLkXJ9W3MHHWKKE7g5oKcYSIrOut0mQEb1sDoblm9h35zUXk5zhwOwlWnM-soCtjeznhmA7yfRNqo-5a3PdwNYapM1vn4=w2400"/>

Creating a Wayland compositor can be a daunting undertaking, often spanning months or even years of effort. This challenging task involves mastering Linux input and graphic APIs, managing graphic buffers, and meticulously implementing numerous Wayland protocols and their respective interfaces.

Fortunately, Louvre simplifies this intricate process by handling all the complex low-level tasks on your behalf. It even provides a default way for managing protocols, enabling you to have a basic but functional compositor from day one and progressively explore and customize its functionality to precisely match your requirements.

## ⭐ Features

* Multi-GPU Support
* Multi-Session Support
* Scene & Views System
* Single, Double or Triple Buffering
* Persistent Clipboard (only for string based MIME types)

## 🧩 Protocols

* Wayland
* XDG Shell
* XDG Decoration
* Presentation Time
* Linux DMA-Buf

## 🖌️ Renderering

Within Louvre, you have the flexibility to either employ your own OpenGL ES 2.0 shaders/programs, use the Louvre::LPainter class for fundamental 2D rendering, or leverage the Louvre::LScene and Louvre::LView system, which manages buffer damage and can also handle input events for you. Additionally, it's possible to combine these three approaches as needed.

## 🔲 Tested Hardware

* Intel (i915 driver)
* AMD (amdgpu driver)
* Nvidia (proprietary and nouveau drivers)
* Mali (lima driver)

## 💬 Client-Compositor Buffer Sharing

* Shared Memory
* Wayland EGL (DMA and GEM flinks)
* Linux DMA-Buf

## 💻 Graphic Backends

* DRM/KMS (with the SRM lib)
* X11 (discontinued since version 1.0.0)

## 🕹️ Input Backends

* Libinput
* X11 (discontinued since version 1.0.0)

## ⏲️ Performance

Louvre offers excellent performance. A benchmark consisting of rendering numerous moving [wl_subsurfaces](https://wayland.app/protocols/wayland#wl_subsurface) (opaque and translucent), in which the [louvre-weston-clone](md_md__examples.html#weston) example compositor was tested, shows that Louvre can maintain a high FPS rate even in complex scenarios. Furthermore, it uses fewer CPU and GPU resources than popular compositors like Weston and Sway.

> If you're interested in the details of how the benchmark operates and would like to try it yourself, please refer to [this link](https://github.com/CuarzoSoftware/Louvre/tree/main/src/benchmark).

Here is a graph illustrating the benchmark results. It displays the average FPS of each compositor rendering 1 to 50 moving surfaces using double buffering on a HiDPI display.

### FPS

<img src="https://lh3.googleusercontent.com/pw/AIL4fc_fcGPw-Yh1zkqxKdfEQucQVXH853Py1YXtTk7jHVACzIaYmYCId07D0hsdJ-FArkERPjJQR2shCc4swA7b1cy9X9EhvFPqLOR_kxV-C1eVQHey2m8=w2400"/>

Most Wayland compositors use a single thread, which drastically slows down their performance when rendering complex scenarios. The reason for this is due to **vertical sync**, where the compositor must wait a few milliseconds before it can swap the framebuffer it just rendered with the one being displayed on screen. This is done in order to synchronize the swapping with the display refresh rate (*vblank*) and avoid the **tearing effect**. When working with a single thread, compositors have "dead times" that prevent them from processing and rendering the content in time for the next frame. That's why they end up skipping a *vblank*, causing their refresh rate to drop in half.
To avoid this problem, Louvre works with multiple threads. Each output (display) renders its content on its own thread, allowing the compositor to continue processing requests and render to other outputs while one is waiting for a vblank. This prevent Louvre compositors from having "dead times" and therefore allows them to maintain a high refresh rate.

### CPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9YhNEf4Rjsqsz49aFtMjyjifDxE9aKgxoOLsfTdJwIu-CqEJr3MJHALX9pgJp05kYJY1z1YBTZjUCQcIAf-gjvRAsumqzEyDm88t1E9SL4aCzaZBo=w2400"/>

The graph on the left displays the raw CPU consumption results, which might suggest that Louvre utilizes more CPU resources. However, this comparison isn't entirely fair, as Louvre's refresh rate is nearly double that of the others (60 FPS vs 30 FPS avg). When we divide the CPU consumption by the frames per second (FPS), as shown in the graph on the right, it becomes evident that Louvre, in fact, uses fewer CPU resources relative to FPS compared to the other compositors.

### GPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc-bzBT_dchcsaVgIOE1iw4iQ2KF_AZ9WItQFXSf2bILxNiaQSpLaXaEkR5p06jb7qdjOqZeYV2m-vHt1KyBed7TH2IQ0jas-lkmxIbcFRAj1w0BojU=w2400"/>

Similarly as with CPU consumption, we can observe that Louvre uses fewer GPU resources relative to FPS than the other compositors.

### 🔨 Upcoming Features

* Touch Events
* Pointer Gestures
* Viewporter
* LView transforms
* LView src rect
* XWayland