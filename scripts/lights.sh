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
		fi
	done
}

setup


for i in 17 27; do
	echo "out" > "$GPIOPATH/gpio${i}/direction"
	echo "0" > "$GPIOPATH/gpio${i}/value"
done

if [ $1 = "on" ]; then
	for i in 17 27; do
		echo "1" | sudo tee "$GPIOPATH/gpio${i}/value" > /dev/null
	done
elif [ $1 = "off" ]; then
	for i in 17 27; do
		echo "0" | sudo tee "$GPIOPATH/gpio${i}/value" > /dev/null
	done
fi
