#!/bin/bash

set -x

GPIOPATH="/sys/class/gpio"
LEFTSHELFGPIO="17"
RIGHTSHELFGPIO="27"

setup()
{ 
	for i in 17 27; do
		if [ ! -e "$GPIOPATH/gpio${i}" ]; then 
			echo "Exporting and configuring gpio${i} (out, low)"
			echo "$i" > "$GPIOPATH/export"
			echo "out" > "$GPIOPATH/gpio${i}/direction"
			echo "0" > "$GPIOPATH/gpio${i}/value"
		fi
	done
}

setup

if [ $1 = "on" ]; then
	for i in 17 27; do
		echo "1" > "$GPIOPATH/gpio${i}/value"
	done
elif [ $1 = "off" ]; then
	for i in 17 27; do
		echo "0" > "$GPIOPATH/gpio${i}/value"
	done
fi
