#!/bin/bash

sudo rm *.txt
array=($(seq 1 5 101))
iters=($(seq 1 1 10))

for iter in "${iters[@]}"
do
	for i in "${array[@]}"
	do
		./bench-louvre.sh $i 10000
		sleep 1
		./bench-weston.sh $i 10000
		sleep 1
	done
	
	mkdir $iter
	sudo chmod 777 *.txt
	mv *.txt ./$iter/
done
