add_definitions("-DACSDK_LOG_MODULE=vinBigDataKeywordDetector")
add_library(VINBIGDATA
        VinBigDataKeywordDetector.cpp)

target_include_directories(VINBIGDATA PUBLIC
    "${VINBIGDATA_SOURCE_DIR}/include"
    "${BDI_LIB}/wuw/include")

target_link_libraries(VINBIGDATA
    acsdkKWDImplementations
    acsdkKWDInterfaces
    AVSCommon
    "${BDI_LIB}/wuw/lib/libwakeup.so"
    "${BDI_LIB}/wuw/lib/libbotan-2.so"
    "${BDI_LIB}/wuw/lib/libtensorflowlite_c.so")

# install target
asdk_install()
