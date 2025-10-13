# 🕹️ Examples {#examples_page}

Louvre provides three illustrative examples that demonstrate its capabilities and various usage scenarios:

* [louvre-views](#louvre-views-example)
* [louvre-weston-clone](#louvre-weston-clone-example)
* [louvre-default](#louvre-default-example)

To run any example, switch to a free TTY session using the `CTRL + ALT + [F1,...,F10]` key combination or execute the `$ sudo chvt N` command, where `N` represents the desired TTY number. Then, proceed to execute the compositor command.

## louvre-views {#louvre-views-example}

The louvre-views example replicates the appearance of macOS X. It showcases the implementation of server-side decorations and demonstrates how to leverage the CZ::LScene and CZ::LView classes to handle tasks such as damage calculation, painting, and input event dispatching on your behalf. This compositor includes a dock for adding applications and presenting minimized windows. Furthermore, it provides the capability to turn windows into fullscreen mode within a designated workspace and navigate between them via a three-finger swipe or by using the keyboard shortcut `Ctrl + Alt + [Left/Right arrow keys]`, closely resembling the behavior seen in macOS.

### Screenshot

<img src="https://lh3.googleusercontent.com/pw/AIL4fc9VCmbRMl7f4ibvQqDrWpmLkXJ9W3MHHWKKE7g5oKcYSIrOut0mQEb1sDoblm9h35zUXk5zhwOwlWnM-soCtjeznhmA7yfRNqo-5a3PdwNYapM1vn4=w2400"/>

<center><i>
The dock icons featured in the screenshot were sourced from <a href="https://macosicons.com/">macOSicons</a>.
</i></center>

### Keyboard Shortcuts

* `Shift + Ctrl + Esc` : Terminates the compositor.
* `Shift + Ctrl + 3` : Captures a screenshot and saves it in your desktop.
* `F1` : Launches weston-terminal.
* `Ctrl + Alt + [F1,...,F10]` : Switches to another TTY.
* `Ctrl + Alt + [Left/Right arrow keys]` : Navigates to the left or right workspace. If supported, you can use three or more fingers on your touchpad.
* `Alt + [mouse over maximize button]` : While hovering the cursor over a server-side decorated toplevel window, press `Alt` to maximize the window instead of setting it to fullscreen mode.
* `Super + Shift + PageUp`: Toggle maximize.
* `Super + Shift + PageDown`: Minimize.
* `Super + Shift + O` : Toggles all outputs.
* `Super + Shift + L` : Toggles the leasable property of all outputs.
* `Super + Shift + V` : Toggles the current output VSync.
* `Super + Shift + T` : Changes the current output transform.
* `Super + Shift + W` : Launches Wofi in run mode.
* `Super + Shift + [Up/Down arrow keys]` : Increments or decrements the current output scale factor by 0.25.
* `Super + Shift + L + [Up/Down arrow keys]` : Modifies the priority of a focused LLayerShell surface.
* `Super + L/R Click` : Starts interactive move/resize for toplevel windows.
* `Super + Ctrl + Scroll` : Zooms in/out.

### Wallpaper

To set the background wallpaper, simply place an image in the following directory: `~/.config/Louvre/wallpaper.jpg`

### Dock Apps

To add apps to the dock, follow these instructions:

1. Create a file named `apps.list` in the `~/.config/Louvre/` directory if it doesn't already exist.

2. Each app entry in the apps.list file should consist of three lines:

* **The application name**: This name will be displayed in the popup when you hover over the app icon and in the top bar when the app is active.
* **Run command**: Specify the command to launch the app. If the command requires environment variables or other parameters, consider creating a Bash script and pointing to that script file.
* **Icon Path**: Provide the full path to the app's icon.

Here's an example format for apps.list:

```
App 1 Name
app1-run-command
/full/path/to/app1_icon.png
App 2 Name
app2-run-command
/full/path/to/app2_icon.png
```

Ensure that there are no empty lines between entries or at the begining/end of the file.

> Note: While using .desktop files is recommended for a more standardized approach, this example simplifies the process for demonstration purposes and avoids the need for linking to additional libraries like GLib or Qt to parse .desktop information and icons.

## louvre-weston-clone {#louvre-weston-clone-example}

The louvre-weston-clone example resembles a Weston-like compositor, showcasing the efficient use of CZ::LPainter. It optimally repaints only the content that requires updating, taking surface damage and opaque/translucent regions into consideration. 

### Screenshot

<img src="https://lh3.googleusercontent.com/pw/AIL4fc_DLN_bkkceKqo6am9k95ydbNruuq0EB9W6srymeTDkME9qYPU9p0tLG5Yklt1QWvyiwzRxaS3UzSjPVRDdd12Zgxc0oElHQF5SJoexvo6srQb_jKQ=w2400"/>

### Keyboard Shortcuts

* `Shift + Ctrl + Esc` : Terminates the compositor.
* `Shift + Ctrl + 3` : Captures a screenshot and saves it to your desktop.
* `F1` : Launches weston-terminal.
* `Ctrl + Alt + [F1,...,F10]` : Switches to another TTY.

### Wallpaper

To set the background wallpaper, simply place an image in the following directory: `~/.config/Louvre/wallpaper.jpg`

## louvre-default {#louvre-default-example}

This example demonstrates Louvre's default behavior: a basic compositor with a white background for user interaction with applications. It uses CZ::LPainter for rendering but in a non-efficient way as it clears and repaints the entire screen each frame.

### Screenshot

<img src="https://lh3.googleusercontent.com/pw/AIL4fc97hD995n2SkAxjZuwS_Lh8zdv_4SojJP_0UL25rLOMpXYjkyT-Qsf656HlBLYnLNQfSCA6O5BlEdzyCzNfDNGWErf7i9U5zpmO6xWOMOTpbK3B88o=w2400"/>

### Keyboard Shortcuts

* `Shift + Ctrl + Esc` : Terminates the compositor.
* `Shift + Ctrl + 3` : Captures a screenshot and saves it to your desktop.
* `F1` : Launches weston-terminal.
* `Ctrl + Alt + [F1,..,F10]` : Switches to another TTY.