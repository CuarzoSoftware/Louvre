# exec <N surfaces> <milliseconds> <seed>
sudo intel_gpu_top -s $2 -o GPU-Louvre_N_$1_MS_$2.txt &
louvre-weston-clone &
export COM_PID=$!
taskset -cp 0 $COM_PID
sleep 2
./LBenchmark $1 $2 FPS-Louvre $3
sudo -E ps -p $COM_PID -o %cpu > CPU-Louvre_N_$1_MS_$2.txt &
sudo -E pkill intel_gpu_top
kill -9 $COM_PID
sleep 1
reset
echo "PID: $COM_PID"
cat FPS-Louvre_N_$1_MS_$2.txt
cat CPU-Louvre_N_$1_MS_$2.txt
cat GPU-Louvre_N_$1_MS_$2.txt
sleep 6