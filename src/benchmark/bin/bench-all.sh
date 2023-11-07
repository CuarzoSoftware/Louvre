#!/bin/bash

sudo rm *.txt
array=($(seq 1 1 50))
iters=($(seq 1 1 10))
timeMS=10000

for iter in "${iters[@]}"
do
	for i in "${array[@]}"
	do
        unset WAYLAND_DISPLAY
		./bench-louvre.sh $i $timeMS $iter
		sleep 1
		./bench-weston.sh $i $timeMS $iter
		sleep 1
	    ./bench-sway.sh $i $timeMS $iter
		sleep 1
	done
	
	mkdir $iter
	sudo chmod 777 *.txt
	mv *.txt ./$iter/
done
