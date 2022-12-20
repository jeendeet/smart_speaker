cmake_minimum_required(VERSION 3.5)
project(OTA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_FLAGS -lmosquittopp)
set(CMAKE_CXX_FLAGS -lcurl)


add_subdirectory(libs)
add_subdirectory(Application)