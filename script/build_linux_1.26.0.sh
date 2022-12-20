#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$(dirname "$SCRIPT_DIR")
echo "BUILD ON DIR: " $ROOT_DIR

LIB_DIR="$ROOT_DIR/sdk-build/libs"

echo "01. Build dependences"
mkdir "$LIB_DIR"
mkdir "$ROOT_DIR/app"
mkdir "$ROOT_DIR/db"
mkdir "$ROOT_DIR/script"
mkdir "$ROOT_DIR/sdk-build"
mkdir "$ROOT_DIR/sdk-install"
mkdir "$ROOT_DIR/sdk-source"
mkdir "$ROOT_DIR/third-party"
mkdir "$ROOT_DIR/tool"

Install pre-requirement
echo "00. Install tools and libs"
sudo apt-get install -y \
	   git gcc cmake openssl clang-format libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev \
	   gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
	   gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools \
	   pulseaudio doxygen libsqlite3-dev libasound2-dev libcurl4-nss-dev

echo "01 - 01. Build aubio"
mkdir -p $LIB_DIR/aubio/lib
mkdir -p $LIB_DIR/aubio/include/aubio
cd "$ROOT_DIR/third-party"
git clone https://github.com/aubio/aubio.git aubio
cd "$ROOT_DIR/third-party/aubio" && make
cd "$ROOT_DIR/third-party/aubio/src" && find . -name "*.h" -exec cp -r --parents {} "$LIB_DIR/aubio/include/aubio" \;
cp "$ROOT_DIR/third-party/aubio/build/src/libaubio.a" "$LIB_DIR/aubio/lib"
sed -i "s/#define AUBIO_FVEC_H/#define AUBIO_FVEC_H\n\n#include \"types.h\"\n/g" "$LIB_DIR/aubio/include/aubio/fvec.h"

echo "01 - 02. Build tensorflow"
mkdir -p $LIB_DIR/tensorlite/lib
mkdir -p $LIB_DIR/tensorlite/include

cd "$ROOT_DIR/third-party"
git clone https://github.com/tensorflow/tensorflow.git tensorflow
cp "$ROOT_DIR/app/patch-tflite/common.c" "$ROOT_DIR/third-party/tensorflow/tensorflow/lite/c/"
mkdir "$ROOT_DIR/third-party/tensorflow/build"
cd "$ROOT_DIR/third-party/tensorflow/build"
cmake ../tensorflow/lite/c
cmake --build . -j
cd "$ROOT_DIR/third-party/tensorflow/" && find tensorflow/lite/c/ -name '*.h' -exec cp -r --parents {} "$LIB_DIR/tensorlite/include" \;
cd "$ROOT_DIR/third-party/tensorflow/tensorflow/lite/c/" && find . -name '*.h' -exec cp -r --parents {} "$LIB_DIR/tensorlite/include" \;
cp "$ROOT_DIR/third-party/tensorflow/build/libtensorflowlite_c.so" "$LIB_DIR/tensorlite/lib"

echo "02. Build wakeup word"
mkdir -p $LIB_DIR/wuw/lib
mkdir -p $LIB_DIR/wuw/include
mkdir "$ROOT_DIR/sdk-build/wuw_build"
cd "$ROOT_DIR/sdk-build/wuw_build"
cmake -DCOMPILE_PC:BOOL=TRUE ../../app/wuw
make -j 8
cd "$ROOT_DIR/app/wuw/" && find . -name 'main_process.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
cd "$ROOT_DIR/app/wuw/" && find . -name 'wuw.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
cd "$ROOT_DIR/app/wuw/" && find . -name 'log.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
cd "$ROOT_DIR/sdk-build/wuw_build" && cp libwakeup.so $LIB_DIR/wuw/lib

# build openssl nghttp2
echo "03. Build  openssl nghttp2"
sudo apt-get -y install build-essential nghttp2 libnghttp2-dev libssl-dev
cd "$ROOT_DIR/third-party/"
wget https://github.com/curl/curl/releases/download/curl-7_67_0/curl-7.67.0.tar.gz
tar xzf curl-7.67.0.tar.gz
cd curl-7.67.0
./configure --with-nghttp2 --prefix=/usr/local --with-ssl
make && sudo make install
sudo ldconfig

# 4. build portaudio
cd "$ROOT_DIR/third-party/"
wget -c http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
tar xf pa_stable_v190600_20161030.tgz
cd portaudio
./configure --without-jack
sed -i 's/CFLAGS = -g/CFLAGS = -g -fPIC/g' Makefile
make -j 8

# 5. build sdk
cd "$ROOT_DIR/sdk-source"
git clone https://github.com/alexa/avs-device-sdk.git avs-device-sdk-1.26.0
cd "$ROOT_DIR/sdk-source/avs-device-sdk-1.26.0"
git checkout v1.26.0
cp -r "$ROOT_DIR/app/patch-1.26.0/*" "$ROOT_DIR/sdk-source/avs-device-sdk-1.26.0"

mkdir "$ROOT_DIR/sdk-build/app_build"
cd "$ROOT_DIR/sdk-build/app_build"

cmake $ROOT_DIR/sdk-source/avs-device-sdk-1.26.0 \
	-DGSTREAMER_MEDIA_PLAYER=ON \
	-DPORTAUDIO=ON \
	-DPORTAUDIO_LIB_PATH=$ROOT_DIR/third-party/portaudio/lib/.libs/libportaudio.a \
	-DPORTAUDIO_INCLUDE_DIR=$ROOT_DIR/third-party/portaudio/include \
	-DCMAKE_BUILD_TYPE=DEBUG \
    -DVINBIGDATA_KEY_WORD_DETECTOR=TRUE \
	-DTARGET_KWD_LIB=VINBIGDATA \
	-DBDI_LIB=$ROOT_DIR/sdk-build/libs

make -j 8
