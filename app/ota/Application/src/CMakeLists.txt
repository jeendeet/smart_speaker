#Bring the headers into the project
include_directories(../include)
# set(CMAKE_CXX_FLAGS -lmosquittopp -lcurl)
# set(CMAKE_CXX_FLAGS -lcurl)
#add the sources using the set command as follows:
set(SOURCES FileWraper.cpp
            HTTPDownload.cpp
            HTTPPing.cpp
            HttpPost.cpp
            MQTTClient.cpp
            OtaClient.cpp
            Session.cpp
            main.cpp
)
add_executable(main ${SOURCES})
TARGET_LINK_LIBRARIES(main mosquittopp curl pthread ssl crypto)
