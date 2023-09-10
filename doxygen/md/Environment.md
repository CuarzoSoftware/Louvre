# Environment Variables

## Debugging

You can harness the **LOUVRE_DEBUG** environment variable, which accepts an integer within the range [0,4], to facilitate the display of debugging information from the library, offering varying levels of verbosity. For further insights, consult the Louvre::LLog documentation.

Debugging a Wayland compositor presents a unique challenge, as traditional terminal-based logging is unavailable. If you find yourself needing to debug your compositor with a tool like GDB, consider SSH-ing into the target machine from another device, even your smartphone, for remote debugging. Note that running the compositor via SSH may encounter issues, necessitating the disabling of libseat. This can be achieved by setting **LOUVRE_ENABLE_LIBSEAT** to 0. Here's an example:

```
$ LOUVRE_ENABLE_LIBSEAT=0 gdb /path/to/your/compositor
```

## Wayland Configuration

By default, Louvre uses the `wayland-0` Unix domain socket for Wayland communication. If you need to customize this socket's name, you can employ the **LOUVRE_WAYLAND_DISPLAY** environment variable. For instance, you can change it to a name like `wayland-1`.

Keep in mind that for clients to successfully establish a connection, you must also update their **WAYLAND_DISPLAY** environment variable to match the chosen value.


## Graphical Backend Configuration

For adjusting parameters related to the graphical backend, including buffer settings (single, double, or triple buffering) or choosing between the Atomic or Legacy DRM API, please consult the [SRM environment variables](https://cuarzosoftware.github.io/SRM/md_md__envs.html).