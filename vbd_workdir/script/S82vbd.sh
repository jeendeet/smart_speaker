#!/bin/bash

# usb enable linkplay board
echo 510 >/sys/class/gpio/export
echo out >/sys/class/gpio/gpio510/direction
echo 1 >/sys/class/gpio/gpio510/value

# start up vbd board

source /bkupgrade/vbd_workdir/script/startup.sh &
