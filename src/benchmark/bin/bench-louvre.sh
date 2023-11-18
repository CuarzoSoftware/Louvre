# exec <N surfaces> <milliseconds> <seed>
sudo intel_gpu_top -s $2 -o GPU-Louvre_N_$1_MS_$2.txt &
taskset --cpu-list 0 louvre-weston-clone &
sleep 2
./LBenchmark $1 $2 FPS-Louvre $3
sudo ps -p `pidof louvre-weston-clone` -o %cpu > CPU-Louvre_N_$1_MS_$2.txt &
sudo -E pkill intel_gpu_top
kill -9 `pidof louvre-weston-clone`
sleep 1
reset
cat FPS-Louvre_N_$1_MS_$2.txt
cat CPU-Louvre_N_$1_MS_$2.txt
cat GPU-Louvre_N_$1_MS_$2.txt
sleep 6
