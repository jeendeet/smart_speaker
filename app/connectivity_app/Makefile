CXX=/usr/bin/arm-linux-gnueabihf-g++
CFLAGS= -std=c++11
ConnectivityServer:
	$(CXX) \
	ConnectivityServer.cpp \
	model/cJSON/cJSON.c \
	model/SHA256/SHA256.cpp \
	model/HttpResponse.cpp \
	model/HandleData.cpp \
	model/HandleSystem.cpp \
	controller/WifiRequest.cpp \
	controller/MetaDataRequest.cpp \
	-o ConnectivityServer
