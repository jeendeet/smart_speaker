SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR=$(dirname "$SCRIPT_DIR")
echo "BUILD ON DIR: " $ROOT_DIR

cd "$ROOT_DIR/sdk-build/app_build"

cmake cmake $ROOT_DIR/sdk-source/vss-device-sdk \
-DGSTREAMER_MEDIA_PLAYER=ON \
-DPORTAUDIO=ON \
-DPKCS11=OFF \
-DPORTAUDIO_LIB_PATH=$ROOT_DIR/third-party/portaudio/lib/.libs/libportaudio.so \
-DPORTAUDIO_INCLUDE_DIR=$ROOT_DIR/third-party/portaudio/include \
-DCMAKE_BUILD_TYPE=DEBUG \
-DVINBIGDATA_KEY_WORD_DETECTOR=TRUE \
-DTARGET_KWD_LIB=VINBIGDATA \
-DBDI_LIB=$ROOT_DIR/sdk-build/libs \
-DCURL_INCLUDE_DIR=$ROOT_DIR/third-party/curl-7.67.0/include/curl \
-DCURL_LIBRARY=$ROOT_DIR/third-party/curl-7.67.0/lib/.libs/libcurl.so 

make -j 8