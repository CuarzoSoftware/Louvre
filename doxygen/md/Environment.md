# ⚙️ Environment Variables

## Debugging

You can harness the **LOUVRE_DEBUG** environment variable, which accepts an integer within the range [0,4], to facilitate the display of debugging information from the library, offering varying levels of verbosity. For further insights, consult the Louvre::LLog documentation.

### GDB

Debugging a Wayland compositor presents a unique challenge, as traditional terminal-based logging is unavailable (there will be a Wayland and X11 backend soon). If you find yourself needing to debug your compositor with a tool like GDB, consider SSH-ing into the target machine from another device, even your smartphone, for remote debugging. Note that running the compositor via SSH may encounter issues, necessitating the disabling of libseat. This can be achieved by setting **LOUVRE_ENABLE_LIBSEAT** to 0. Here's an example:

```
$ LOUVRE_ENABLE_LIBSEAT=0 gdb /path/to/your/compositor
```

## Wayland Configuration

By default, Louvre uses the `wayland-0` Unix domain socket for Wayland communication. If you need to customize this socket's name, you can employ the **LOUVRE_WAYLAND_DISPLAY** environment variable. For instance, you can change it to a name like `wayland-1`.

Keep in mind that for clients to successfully establish a connection, you must also update the **WAYLAND_DISPLAY** environment variable to match the chosen value.

## Backends Configuration

  - **LOUVRE_BACKENDS_PATH**: Directory containing Louvre backends. The directory structure must include two subdirectories as follows:

  ```
  backends
  |-- graphic
      |-- drm.so
      |-- another_graphic_backend.so
  |-- input
      |-- libinput.so
      |-- another_input_backend.so
  ```

  - **LOUVRE_GRAPHIC_BACKEND**: Name of the graphic backend to load, excluding the `.so` extension, for example, `drm`.

  - **LOUVRE_INPUT_BACKEND**: Name of the input backend to load, excluding the `.so` extension, for example, `libinput`.

If Louvre encounters any issues while loading the specified backend configurations, it will automatically revert to the default settings. You can configure the default backends and paths by using the `libdir` Meson option and by modifying the `meson_options.txt` file during the Louvre build process.

## DRM Graphic Backend Configuration {#graphic}

For adjusting parameters related to the DRM graphic backend, including buffering settings (single, double, or triple buffering) or choosing between the Atomic or Legacy DRM API, please consult the [SRM environment variables](https://cuarzosoftware.github.io/SRM/md_md__envs.html).

### Recommended Settings

The SRM backend uses double buffering by default.

For smoother performance try triple buffering:

  - **SRM_RENDER_MODE_ITSELF_FB_COUNT**=3
  - **SRM_RENDER_MODE_DUMB_FB_COUNT**=3

For snappier cursor updates, force the legacy DRM API (asynchronous hardware cursor updates):

  - **SRM_FORCE_LEGACY_API**=1

> Starting from Louvre version 1.2 the SRM backend defaults to using the legacy API. To enable the atomic API instead, simply set **SRM_FORCE_LEGACY_API**=0.