#!/bin/bash
set -e 

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$(dirname "$SCRIPT_DIR")
echo "BUILD ON DIR: " $ROOT_DIR

# Build Onboarding wifi
echo "Build Onboarding wifi"
cd "$ROOT_DIR/app/connectivity_app"
rm "$ROOT_DIR/app/connectivity_app/ConnectivityServer"
make
cp ConnectivityServer "$ROOT_DIR/vbd_workdir/onbo_wifi/"
cp -r script "$ROOT_DIR/vbd_workdir/onbo_wifi/"
cp -r config "$ROOT_DIR/vbd_workdir/onbo_wifi/"

# Build Onboarding bluetooth
echo "Build Onboarding bluetooth"
cd "$ROOT_DIR/app/connectivity_app_bluetooth"
make
cp connected_app_ble "$ROOT_DIR/vbd_workdir/onbo_ble/"

# Build Root App
echo "Build Root App"
cd "$ROOT_DIR/app/vRootApp"
/usr/bin/arm-linux-gnueabihf-g++ vRootApp.cpp -o vRootApp
cp vRootApp "$ROOT_DIR/vbd_workdir/vRootApp/"
cp config.txt "$ROOT_DIR/vbd_workdir/vRootApp/"

# Build bluetooth sink
echo "Build bluetooth sink"
cd "$ROOT_DIR/app/bluetooth_sink"
make
cp app_manager "$ROOT_DIR/vbd_workdir/vbd_blue/"
cp app_avk "$ROOT_DIR/vbd_workdir/vbd_blue/"

# Build AVS
echo "Build AVS"
cd "$ROOT_DIR"
find sdk-build -name "*.so" -exec scp {} "$ROOT_DIR/vbd_workdir/lib/libAVSRelease/" \;
cp -r "$ROOT_DIR/sdk-build/app_build/SampleApp" "$ROOT_DIR/vbd_workdir/vbd_avs/"
cp "$ROOT_DIR/vbd_workdir/vbd_avs/AlexaClientSDKConfig.json" "$ROOT_DIR/vbd_workdir/vbd_avs/SampleApp/src/"
cp -r "$ROOT_DIR/vbd_workdir/vbd_avs/inputs" "$ROOT_DIR/vbd_workdir/vbd_avs/SampleApp/src/"
cp -r "$ROOT_DIR/vbd_workdir/vbd_avs/sdk-install" "$ROOT_DIR/vbd_workdir/vbd_avs/SampleApp/src/"
mkdir "$ROOT_DIR/vbd_workdir/vbd_avs/SampleApp/src/database"
