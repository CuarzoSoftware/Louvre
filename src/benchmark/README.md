# LBenchmark

The LBenchmark entails a Wayland client that generates a maximized toplevel window containing numerous child wl_subsurfaces. These surfaces are both opaque and translucent, which move in a pseudo-random manner across the screen. The randomness is determined by a seed that remains constant across the testing of three different compositors. Furthermore, the movement of these surfaces is time-dependent, ensuring that all three compositors render the same content over the course of the benchmark, regardless of potential variations in refresh rates.

## Build
To build the project, navigate to the ./LBenchmark directory and employ qmake. Subsequently, locate the produced LBenchmark executable within the ./bin directory.

## Run
Ensure that the louvre-weston-clone, weston, and sway compositors are installed on your system and avaliable in the **PATH** env. Switch to an available TTY and execute the bench-all.sh script (please note that this process may require a few hours to complete).

## Graphs
Once the benchmark concludes, duplicate the created folders (labeled as 1, 2, 3, ..., etc.) in ./bin into a new directory. Transfer this directory into the ./graphs directory and launch the Jupyter notebook. Replace the folder name variable in the notebook with the name of your newly copied folder, and execute the notebook to generate the desired graphs.