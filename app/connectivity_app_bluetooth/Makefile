objects = ConnectedAppBLE
all: $(objects)

CXX=/usr/bin/arm-linux-gnueabihf-gcc
#CFLAGS= -std=c++11
INCLUDE= -Ilib_ble/include -Ilibbsa/include -Isyn_ble/include
LIBRARY= -Llibbsa/build/arm/sharedlib -lbsa
ADD_LIBRARY= -lpthread -lcurl -lmosquitto
ADD_FLAGS= -DBLE_INCLUDED -DBTA_GATT_INCLUDED
ConnectedAppBLE:
	$(CXX) \
	$(CFLAGS) \
	$(INCLUDE) \
	syn_ble/source/app_ble_main.c \
	syn_ble/source/app_ble.c \
	syn_ble/source/app_ble_server.c \
	syn_ble/source/app_handle.c \
	syn_ble/source/app_curl_request.c \
	lib_ble/source/app_disc.c \
	lib_ble/source/app_dm.c \
	lib_ble/source/app_manager.c \
	lib_ble/source/app_services.c \
	lib_ble/source/app_utils.c \
	lib_ble/source/app_xml_param.c \
	lib_ble/source/app_xml_utils.c \
	lib_ble/source/app_mgt.c \
	lib_ble/source/nanoxml.c \
	$(LIBRARY) \
	$(ADD_LIBRARY) \
	$(ADD_FLAGS) \
	-o connected_app_ble
