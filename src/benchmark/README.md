# LBenchmark Overview

The LBenchmark involves a Wayland client that generates a white maximized toplevel window containing numerous child wl_subsurfaces. These surfaces are both opaque and translucent, moving in a pseudo-random manner across the screen. The randomness is determined by a seed that remains constant throughout the testing of three different compositors. Additionally, the movement of these surfaces is calculated using a sinusoidal function with a phase that is time-dependent (ranging from 0 to the duration of a run). This ensures that all three compositors render the same content over the course of the benchmark, regardless of potential variations in refresh rates.

<img src="https://lh3.googleusercontent.com/pw/ADCreHdzBr06L83S-_R_hnVsinedwAP6sZGA0JZS4MVUwI2xlckBnNnIaNXI492no5dPEVLP4lV5jZ9jj7vNXG29L7A0Vp-jzdBGXqOgH349nNYm4lOfdEI=w2400"/>

All moving subsurfaces have a shm buffer attached and are marked as damaged only at the beginning of a benchmark run, not in subsequent frames. The objective is to measure how well and efficiently each compositor manages to repaint what needs to be repainted. There is also a unique subsurface displayed on top that changes color and is, in fact, damaged in each frame so that the buffer coping mechanisms of each compositor can be considered.

## CPU Measurement

Each compositor is executed using the taskset command and assigned to work exclusively with CPU core number 0. To measure CPU consumption, the `ps -p PID -o %cpu` command is invoked just before terminating the compositor. According to the [ps manual](https://man7.org/linux/man-pages/man1/ps.1.html), this command returns *"the CPU time used divided by the time the process has been running (cputime/realtime ratio), expressed as a percentage."*

## GPU Measurement

GPU consumption is assessed using the `intel_gpu_top` command, configured with the -s option, where the refresh period is set to match the duration of each run in milliseconds.

```bash
Usage: intel_gpu_top [parameters]

	The following parameters are optional:

	[-h]            Show this help text.
	[-J]            Output JSON formatted data.
	[-l]            List plain text data.
	[-o <file|->]   Output to specified file or '-' for standard out.
	[-s <ms>]       Refresh period in milliseconds (default 1000ms).
	[-L]            List all cards.
	[-d <device>]   Device filter, please check manual page for more details.
```

## FPS Measurement

FPS is determined by summing the number of frame callbacks returned by the compositor for the maximized toplevel surface and dividing it by the duration of each run.

## Averaging

The benchmark is executed 10 times for each compositor, each time employing a different seed. The results are then averaged within the Jupyter notebook.

## Building the benchmark client

To build the client application, navigate to the `./LBenchmark` directory and employ:

```bash
$ meson setup build
$ cd build
$ meson compile
```

Subsequently, locate the produced `LBenchmark` executable within the `./bin` directory.

## Run

Ensure that the `louvre-weston-clone`, `weston`, and `sway` compositors are installed on your system and avaliable in the **PATH** env. Switch to an available TTY and execute the `bench-all.sh` script (please note that this process may require a few hours to complete).

## Graphs

Upon completion of the benchmark, copy the folders created (labeled as 1, 2, 3, ..., etc.) in the `./bin` directory into a new folder. Move this folder into the `./graphs` directory and initiate the Jupyter notebook. Subsequently, update the folder name variable and title in the function call at the end of the notebook with the name of your newly created folder, like so: `graphs('your_folder', 'Add a custom title')`. Execute the notebook to generate the desired graphs.