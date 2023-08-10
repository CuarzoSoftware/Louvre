
<img style="position:relative;margin:0px;padding:0;top:40px" src="https://i.imgur.com/cCT9KwN.png" width="104"/>
<h1 style="margin-top:0px;padding-top:0px">Louvre</h1>

Louvre is a C++ library that aims to simplify the process of creating Wayland compositors by reducing its learning curve and thus allowing developers to focus on the creative and unique features of their compositors.

The API documentation, installation guide, and tutorial can be found [**here**](https://cuarzosoftware.github.io/Louvre/index.html).

## Contact

If you come across any bugs or have questions or suggestions, please feel free to email me at <ehopperdietzel@gmail.com>.

## ⏲️ Performance

Louvre offers excellent performance. A benchmark consisting of rendering numerous moving wl_subsurfaces (opaque and translucent), in which the **louvre-weston-clone** example compositor was tested, shows that Louvre can maintain a high FPS rate even in complex scenarios. Furthermore, it uses fewer CPU and GPU resources than popular compositors like Weston and Sway.

*The code of the benchmark can be found in ```Louvre/src/benchmark```*.

Here is a graph illustrating the benchmark results. It displays the average FPS of each compositor rendering 1 to 50 moving surfaces.

### FPS

<img src="https://lh3.googleusercontent.com/pw/AIL4fc_fcGPw-Yh1zkqxKdfEQucQVXH853Py1YXtTk7jHVACzIaYmYCId07D0hsdJ-FArkERPjJQR2shCc4swA7b1cy9X9EhvFPqLOR_kxV-C1eVQHey2m8=w2400"/>

Most Wayland compositors use a single thread, which drastically slows down their performance when rendering complex scenarios. The reason for this is due to **vertical sync**, where the compositor must wait a few milliseconds before it can swap the framebuffer it just rendered with the one being displayed on screen. This is done in order to synchronize the swapping with the monitor refresh rate (*vblank*) and avoid the **tearing effect**. When working with a single thread, compositors have "dead times" that prevent them from processing and rendering the content in time for the next frame. That's why they end up skipping one frame, causing their refresh rate to drop in half.
To avoid this problem, Louvre works with multiple threads. Each output (monitor) renders its content on its own thread, allowing the compositor to continue processing requests and render to other outputs while one is busy. This prevents the compositor from having dead times and therefore allows it to maintain a high refresh rate.

### CPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9YhNEf4Rjsqsz49aFtMjyjifDxE9aKgxoOLsfTdJwIu-CqEJr3MJHALX9pgJp05kYJY1z1YBTZjUCQcIAf-gjvRAsumqzEyDm88t1E9SL4aCzaZBo=w2400"/>

On the left are the raw CPU consumption results, which seemingly indicate that Louvre uses more CPU (which is obvious, given its higher FPS rendering). However, when we normalize it to the FPS (graph on the right), we can observe that, in fact, it employs less CPU than the other compositors (relative to FPS).

### GPU Consumption

<img src="https://lh3.googleusercontent.com/pw/AIL4fc-bzBT_dchcsaVgIOE1iw4iQ2KF_AZ9WItQFXSf2bILxNiaQSpLaXaEkR5p06jb7qdjOqZeYV2m-vHt1KyBed7TH2IQ0jas-lkmxIbcFRAj1w0BojU=w2400"/>

Similarly as with CPU consumption, we can observe that Louvre uses fewer GPU resources than the other compositors.

## 🖌️ Renderering

Within Louvre, you have the flexibility to either employ your own OpenGL ES 2.0 shaders/programs, use the LPainter class for fundamental 2D rendering, or leverage the scene and views system, which manages buffer damage and can also handle input events for you. Additionally, it's possible to combine these three approaches as needed.

## 🔲 Tested Hardware

* Intel (i915 driver)
* AMD (amdgpu driver)
* Nvidia (proprietary and nouveau drivers)
* Mali (lima driver)

Please be aware that specific older nouveau drivers, due to their absence of multithreading support, could lead to crashes. In such instances, it's advisable to consider using proprietary Nvidia drivers.

## 🧩 Protocols

* Wayland
* XDG Shell
* XDG Decoration Manager
* Presentation Time
* Linux DMA-Buf
* WP Pointer Gestures (WIP)

## 💬 Buffer Sharing

* Shared Memory
* Wayland EGL (GEM flinks are supported)
* Linux DMA-Buff

## 👴 Compatible Clients

* GTK
* Ozone (Chromium)
* Qt
* SDL
* EFL

## 💻 Graphic Backends

* DRM/KMS
* X11 (removed since version 1.0.0)

## 🕹️ Input Backends

* Libinput
* X11 (removed since version 1.0.0)

