# ⚙️ Environment Variables {#EnvironmentPage}

## Debugging

* **LOUVRE_DEBUG**: Enables debugging messages. Accepts an integer in the range [0-4]. For details, consult the Louvre::LLog documentation.

## Wayland Socket

* **LOUVRE_WAYLAND_DISPLAY**: Socket name for communicating with clients. Defaults to `wayland-2`.

* **WAYLAND_DISPLAY**: If set before launching the compositor, Louvre will attempt to load the Wayland backend.

## Backends Configuration

  - **LOUVRE_BACKENDS_PATH**: Directory containing Louvre backends. The directory structure must include two subdirectories as follows:

  ```
  your_dir
  |-- graphic
      |-- drm.so
      |-- another_graphic_backend.so
  |-- input
      |-- libinput.so
      |-- another_input_backend.so
  ```

  - **LOUVRE_GRAPHIC_BACKEND**: Name of the graphic backend to load, excluding the `.so` extension, for example, `drm`.

  - **LOUVRE_INPUT_BACKEND**: Name of the input backend to load, excluding the `.so` extension, for example, `libinput`.

## DRM Graphic Backend Configuration {#graphic}

For adjusting parameters related to the DRM graphic backend, including buffering settings (single, double, or triple buffering) or choosing between the Atomic or Legacy DRM API, please consult the [SRM environment variables](https://cuarzosoftware.github.io/SRM/md_md__envs.html).
