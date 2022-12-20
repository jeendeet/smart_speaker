echo 510 >/sys/class/gpio/export
echo out >/sys/class/gpio/gpio510/direction
echo 1 >/sys/class/gpio/gpio510/value