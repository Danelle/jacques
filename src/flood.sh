#!/bin/bash

# 不再是测试，而是攻击

for i in {1..10}
do
	./batch.sh &
	sleep 0.6
done
