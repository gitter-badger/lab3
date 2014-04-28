#!/bin/bash
while true; do
	ipcs -s
	ipcs -s -i ${1}
	sleep 1
	clear
done
