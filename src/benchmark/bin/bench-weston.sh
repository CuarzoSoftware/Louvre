# exec <N surfaces> <milliseconds> <seed>
sudo intel_gpu_top -s $2 -o GPU-Weston_N_$1_MS_$2.txt &
taskset --cpu-list 0 weston
sleep 2
./LBenchmark $1 $2 FPS-Weston $3
sudo ps -p `pidof weston` -o %cpu > CPU-Weston_N_$1_MS_$2.txt &
sudo -E pkill intel_gpu_top
pkill weston
sleep 1
reset
cat FPS-Weston_N_$1_MS_$2.txt
cat CPU-Weston_N_$1_MS_$2.txt
cat GPU-Weston_N_$1_MS_$2.txt
sleep 6
