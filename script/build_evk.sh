#!/bin/bash
set -e 

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$(dirname "$SCRIPT_DIR")
echo "BUILD ON DIR: " $ROOT_DIR

LIB_DIR="$ROOT_DIR/sdk-build/libs"

echo "01. Build dependences"
mkdir -p "$ROOT_DIR/sdk-build"
mkdir -p "$ROOT_DIR/sdk-source"
mkdir -p "$ROOT_DIR/third-party"
mkdir -p "$LIB_DIR"

# Install pre-requirement
echo "00. Install tools and libs"
apt-get -y install \
 	git gcc cmake build-essential libsqlite3-dev libcurl4-openssl-dev libfaad-dev \
 	libssl-dev libicu-dev libsoup2.4-dev libgstreamer-plugins-base1.0-dev libxml2-dev libgcrypt20-dev libgstreamer-plugins-bad1.0-dev \
 	libnghttp2-dev nghttp2 gstreamer1.0-plugins-good libasound2-dev doxygen  

# echo "01 - 01. Build aubio"
# mkdir -p $LIB_DIR/aubio/lib
# mkdir -p $LIB_DIR/aubio/include/aubio
# cd "$ROOT_DIR/third-party"
# git clone https://github.com/aubio/aubio.git aubio
# # cd "$ROOT_DIR/third-party/aubio" && make
# mkdir -p $ROOT_DIR/third-party/aubio/build/
# mkdir -p $ROOT_DIR/third-party/aubio/build/src/
# cp "/usr/lib/arm-linux-gnueabihf/libaubio.a" "$ROOT_DIR/third-party/aubio/build/src/"
# cd "$ROOT_DIR/third-party/aubio/src" && find . -name "*.h" -exec cp -r --parents {} "$LIB_DIR/aubio/include/aubio" \;
# cp "$ROOT_DIR/third-party/aubio/build/src/libaubio.a" "$LIB_DIR/aubio/lib"
# sed -i "s/#define AUBIO_FVEC_H/#define AUBIO_FVEC_H\n\n#include \"types.h\"\n/g" "$LIB_DIR/aubio/include/aubio/fvec.h"

# echo "01 - 02. Build tensorflow"
# mkdir -p $LIB_DIR/tensorlite/lib
# mkdir -p $LIB_DIR/tensorlite/include

# cd "$ROOT_DIR/third-party"
# wget -O tensorflow.zip https://github.com/tensorflow/tensorflow/archive/v2.6.0.zip
# unzip tensorflow.zip
# mv tensorflow-2.6.0 tensorflow
# # git clone https://github.com/tensorflow/tensorflow.git tensorflow
# # cp "$ROOT_DIR/app/patch-tflite/common.c" "$ROOT_DIR/third-party/tensorflow/tensorflow/lite/c/"
# mkdir -p "$ROOT_DIR/third-party/tensorflow/build"
# cd "$ROOT_DIR/third-party/tensorflow/build"
# cmake -DTFLITE_ENABLE_XNNPACK=OFF -DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain.cmake ../tensorflow/lite/c   
# cmake --build . -j2
# cd "$ROOT_DIR/third-party/tensorflow/" && find tensorflow/lite/c/ -name '*.h' -exec cp -r --parents {} "$LIB_DIR/tensorlite/include" \;
# cd "$ROOT_DIR/third-party/tensorflow/tensorflow/lite/c/" && find . -name '*.h' -exec cp -r --parents {} "$LIB_DIR/tensorlite/include" \;
# cp "$ROOT_DIR/third-party/tensorflow/build/libtensorflowlite_c.so" "$LIB_DIR/tensorlite/lib"

echo "02. Build wakeup word"
mkdir -p $LIB_DIR/wuw/lib
mkdir -p $LIB_DIR/wuw/include
mkdir "$ROOT_DIR/sdk-build/wuw_build"
cd "$ROOT_DIR/sdk-build/wuw_build"
# cmake -DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain.cmake -DCMAKE_CXX_STANDARD_LIBRARIES="-lfftw3 -lfftw3f -lm" -DCOMPILE_PC:BOOL=TRUE ../../app/wuw
# make -j 4
# cd "$ROOT_DIR/app/wuw/" && find . -name 'main_process.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
# cd "$ROOT_DIR/app/wuw/" && find . -name 'wuw.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
# cd "$ROOT_DIR/app/wuw/" && find . -name 'log.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
# cd "$ROOT_DIR/sdk-build/wuw_build" && cp libwakeup.so $LIB_DIR/wuw/lib

# cd "$ROOT_DIR/app/wuw/" && find . -name 'main_process.h' -exec cp --parents {} $LIB_DIR/wuw/include \;
cd "$ROOT_DIR/app/wuw/" && cp -r "./include" $LIB_DIR/wuw/
cd "$ROOT_DIR/app/wuw/" && cp -r "./lib" $LIB_DIR/wuw/ 


## build openssl nghttp2
echo "03. Build  openssl nghttp2"
cd "$ROOT_DIR/third-party/"
wget https://github.com/curl/curl/releases/download/curl-7_67_0/curl-7.67.0.tar.gz
tar xzf curl-7.67.0.tar.gz
cd curl-7.67.0
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf --with-ssl
# ./configure --with-nghttp2 --prefix=/usr/local --with-ssl
make && make install
ldconfig

# 4. build portaudio
cd "$ROOT_DIR/third-party/"
wget -c http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
tar xf pa_stable_v190600_20161030.tgz
cd portaudio
./configure --host=arm-linux-gnueabihf --prefix=/usr/arm-linux-gnueabihf --without-jack && make
sed -i 's/CFLAGS = -g/CFLAGS = -g -fPIC/g' Makefile
make -j 4

# 5. build sdk
# copy patch build wuw
cp -r $ROOT_DIR/app/patch-1.26.0/* $ROOT_DIR/sdk-source/vss-device-sdk
mkdir -p "$ROOT_DIR/sdk-build/app_build"
cd "$ROOT_DIR/sdk-build/app_build"

cmake cmake -lfftwf -lfftw3 -lfftw3f -lfftw3l -lm $ROOT_DIR/sdk-source/vss-device-sdk \
 -DCMAKE_TOOLCHAIN_FILE=/tmp/toolchain.cmake \
 -DGSTREAMER_MEDIA_PLAYER=ON \
 -DPORTAUDIO=ON \
 -DBLUETOOTH_ENABLED=ON \
 -DPKCS11=OFF \
 -DPORTAUDIO_LIB_PATH=$ROOT_DIR/third-party/portaudio/lib/.libs/libportaudio.so \
 -DPORTAUDIO_INCLUDE_DIR=$ROOT_DIR/third-party/portaudio/include \
 -DCMAKE_BUILD_TYPE=DEBUG \
 -DVINBIGDATA_KEY_WORD_DETECTOR=TRUE \
 -DTARGET_KWD_LIB=VINBIGDATA \
 -DBDI_LIB=$ROOT_DIR/sdk-build/libs \
 -DCURL_INCLUDE_DIR=$ROOT_DIR/third-party/curl-7.67.0/include/curl \
 -DCURL_LIBRARY=$ROOT_DIR/third-party/curl-7.67.0/lib/.libs/libcurl.so

make -j 4
