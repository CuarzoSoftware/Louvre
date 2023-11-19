# exec <N surfaces> <milliseconds> <seed>
sudo intel_gpu_top -s $2 -o GPU-Sway_N_$1_MS_$2.txt &
taskset --cpu-list 0 sway &
sleep 2
export WAYLAND_DISPLAY=wayland-1
./LBenchmark $1 $2 FPS-Sway $3
sudo ps -p `pidof sway` -o %cpu > CPU-Sway_N_$1_MS_$2.txt &
sudo -E pkill intel_gpu_top
pkill sway
sleep 1
reset
cat FPS-Sway_N_$1_MS_$2.txt
cat CPU-Sway_N_$1_MS_$2.txt
cat GPU-Sway_N_$1_MS_$2.txt
sleep 6
