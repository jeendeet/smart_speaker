objects = AppManager AppAvk
all: $(objects)

CXX=/usr/bin/arm-linux-gnueabihf-g++
#CFLAGS= -std=c++11
INCLUDE= -Ilib_ble/include -Ilibbsa/include
LIBRARY= -Llibbsa/build/arm/sharedlib -lbsa
ADD_LIBRARY= -lpthread -lasound -lmosquitto
ADD_FLAGS= -DPCM_ALSA_OPEN_BLOCKING -DPCM_ALSA
AppManager:
	$(CXX) \
	$(CFLAGS) \
	$(INCLUDE) \
	syn_ble/source/app_manager_main.c \
	lib_ble/source/app_avk.c \
	lib_ble/source/app_disc.c \
	lib_ble/source/app_dm.c \
	lib_ble/source/app_manager.c \
	lib_ble/source/app_services.c \
	lib_ble/source/app_wav.c \
	lib_ble/source/app_utils.c \
	lib_ble/source/app_xml_param.c \
	lib_ble/source/app_xml_utils.c \
	lib_ble/source/app_mgt.c \
	lib_ble/source/nanoxml.c \
	lib_ble/source/app_mqtt.c \
	lib_ble/source/cJSON.c \
	lib_ble/source/app_json.cpp \
	$(LIBRARY) \
	$(ADD_LIBRARY) \
	$(ADD_FLAGS) \
	-o app_manager

AppAvk:
	$(CXX) \
	$(CFLAGS) \
	$(INCLUDE) \
	syn_ble/source/app_avk_main.c \
	lib_ble/source/app_avk.c \
	lib_ble/source/app_disc.c \
	lib_ble/source/app_dm.c \
	lib_ble/source/app_manager.c \
	lib_ble/source/app_services.c \
	lib_ble/source/app_wav.c \
	lib_ble/source/app_utils.c \
	lib_ble/source/app_xml_param.c \
	lib_ble/source/app_xml_utils.c \
	lib_ble/source/app_mgt.c \
	lib_ble/source/nanoxml.c \
	lib_ble/source/app_mqtt.c \
	lib_ble/source/cJSON.c \
	lib_ble/source/app_json.cpp \
	$(LIBRARY) \
	$(ADD_LIBRARY) \
	$(ADD_FLAGS) \
	-o app_avk
