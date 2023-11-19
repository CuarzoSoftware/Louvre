# ⚙️ Environment Variables

## Debugging

You can harness the **LOUVRE_DEBUG** environment variable, which accepts an integer within the range [0,4], to facilitate the display of debugging information from the library, offering varying levels of verbosity. For further insights, consult the Louvre::LLog documentation.

### GDB

Debugging a Wayland compositor presents a unique challenge, as traditional terminal-based logging is unavailable. If you find yourself needing to debug your compositor with a tool like GDB, consider SSH-ing into the target machine from another device, even your smartphone, for remote debugging. Note that running the compositor via SSH may encounter issues, necessitating the disabling of libseat. This can be achieved by setting **LOUVRE_ENABLE_LIBSEAT** to 0. Here's an example:

```
$ LOUVRE_ENABLE_LIBSEAT=0 gdb /path/to/your/compositor
```

## Wayland Configuration

By default, Louvre uses the `wayland-0` Unix domain socket for Wayland communication. If you need to customize this socket's name, you can employ the **LOUVRE_WAYLAND_DISPLAY** environment variable. For instance, you can change it to a name like `wayland-1`.

Keep in mind that for clients to successfully establish a connection, you must also update the **WAYLAND_DISPLAY** environment variable to match the chosen value.

## Backends Configuration

  - **LOUVRE_BACKENDS_PATH**: Path to the directory containing Louvre backends, e.g., `/usr/etc/Louvre/backends`.

  - **LOUVRE_GRAPHIC_BACKEND**: Name of the graphic backend to load, e.g., `libLGraphicBackendDRM.so`.

  - **LOUVRE_INPUT_BACKEND**: Name of the input backend to load, e.g., `libLInputBackendLibinput.so`.

If Louvre encounters issues loading the specified backend configurations, it will automatically fallback to the default settings.

## DRM Graphic Backend Configuration {#graphic}

For adjusting parameters related to the DRM graphic backend, including buffering settings (single, double, or triple buffering) or choosing between the Atomic or Legacy DRM API, please consult the [SRM environment variables](https://cuarzosoftware.github.io/SRM/md_md__envs.html).

### Recommended Settings

The SRM backend uses double buffering by default.

For smoother performance on relatively powerful hardware, try triple buffering:

  - **SRM_RENDER_MODE_ITSELF_FB_COUNT**=3

If you have very limited hardware, consider using single buffering (no v-sync, potential glitches):

  - **SRM_RENDER_MODE_ITSELF_FB_COUNT**=1

For snappier cursor updates, force the legacy DRM API (asynchronous hardware cursor updates):

  - **SRM_FORCE_LEGACY_API**=1

> Keep in mind that using the legacy API may lead to performance problems with certain drivers, like some proprietary Nvidia drivers.



