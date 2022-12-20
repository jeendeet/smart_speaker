# Description
vRootApp check processes of AVS / Connected App / Connected-BLE App / Mosquitto Service
vRootApp controls these processes and restart its if it's killed or died.

config.txt saves path of folder, library of theses processes, we can change its if we change path of library or bin file.

# Build vRootApp
$ usr/bin/arm-linux-gnueabihf-g++ vRootApp.cpp -o vRootApp

# Run vRootApp
$ cd {{folder contains config.txt}} && ./vRootApp

# Terminate vRootApp and others processes
$ killall vRootApp 
