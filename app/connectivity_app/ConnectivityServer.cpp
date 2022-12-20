/* g++ ConnectivityServer.cpp model/cJSON/cJSON.c model/HttpResponse.cpp model/HandleData.cpp model/HandleSystem.cpp controller/WifiRequest.cpp -o ConnectivityServer.o -lm*/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <time.h>
#include <vector>

#include "model/cJSON/cJSON.h"
#include "model/HttpResponse.h"
#include "model/HandleData.h"
#include "model/HandleSystem.h"
#include "controller/WifiRequest.h"
#include "controller/MetaDataRequest.h"

#include "config/define.h"
#include "config/log.h"

/* Set log level */
int log_level = LOG_LEVEL;
int main(int argc, char *argv[])
{
	setLogLevel(log_level);
	if (argc != 2)
	{
		LOGE("missing port");
		exit(1);
	} 
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof(client_addr);
	int fd_server, fd_client;
	char buf[BUFFER_SIZE];
	int fdimg;
	int on = 1;

	/* create socket server*/
	LOGI("Create socket");
	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server < 0)
	{
		LOGE("Cannot create socket");
		exit(1);
	}

	/* set socket option */
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));

	/* bind socket */
	LOGI("Bind socket");
	if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
	{
		LOGE("Cannot bind socket");
		close (fd_server);
		exit(1);
	}

	/* listen socket */
	LOGI("Listen socket");
	if (listen(fd_server, 10) == -1)
	{
		LOGE("Cannot listen socket");
		close (fd_server);
		exit(1);
	}

	/* loop to check buffer */
	while(1)
	{
		fd_client = accept (fd_server, (struct sockaddr *) &client_addr, &sin_len);
		if (fd_client == -1)
		{
			LOGE("Connection failed....");
			continue;
		}

		LOGI("Got client connection....");
		
		if (!fork())
		{
			/* child process */
			close (fd_server);
			memset (buf, 0, BUFFER_SIZE);
			read (fd_client, buf, BUFFER_SIZE);
			std::string param_str, buf_str;
			cJSON *json;
			
			LOGD("Message from client : %s", buf);
			buf_str = HandleData::Instance()->convert_to_string((char*)buf);
			if (!strncmp (buf, "GET /wifi-connect-request", 25))
			{
				LOGI("GET /wifi-connect-request");
				/* get json message data from buf from { charater to end string */
				std::string ssid, psk;
				param_str = HandleData::Instance()->get_str_between_two_str(buf_str, "/wifi-connect-request?", " HTTP/");

				char *input = (char *)param_str.c_str();
				char *output = (char *)malloc(strlen(input)*2);
				HandleData::Instance()->urldecode2(output, input);
				LOGD("Decoded string: %s\n", output);
				param_str = HandleData::Instance()->convert_to_string(output);
				free(output);

				std::vector<std::string> params;
				HandleData::Instance()->split(param_str, "&", params);
				if (param_str.find("ssid") == std::string::npos) {
					char *response = HttpResponse::Instance()->create_response(
						(char*)HTTP_400_ERROR,
						(char*)SSID_NOT_EXISTED
					);
					write(fd_client, response, strlen(response));
					LOGD("Response : %s", response);
					close (fd_client);
					LOGI("Closing...");
					exit(1);
				}
				if (param_str.find("psk") == std::string::npos) {
					psk = ""; /* wifi no password */
					/* comment for case wifi no password
					char *response = HttpResponse::Instance()->create_response(
						(char*)HTTP_400_ERROR,
						(char*)PSK_NOT_EXISTED
					);
					write(fd_client, response, strlen(response));
					LOGD("Response : %s", response);
					close (fd_client);
					LOGI("Closing...");
					exit(1);
					*/
				}
				
				for (auto it = params.begin(); it != params.end(); it++) {
					std::string temp_params = *it;
					if (temp_params.find("ssid=") != std::string::npos) {
						ssid = HandleData::Instance()->get_str_between_two_str(*it, "ssid=", "\0");
					}
					if (temp_params.find("psk=") != std::string::npos) {
						psk = HandleData::Instance()->get_str_between_two_str(*it, "psk=", "\0");
					}
				}
				/* request wifi connection */
				LOGI("request wifi connection");
				std::string output_req = WifiRequest::Instance()->wifi_connect_request(
					(char*)ssid.c_str(), (char*)psk.c_str()
				);
				char *response = WifiRequest::Instance()->wifi_checking(output_req);
				write(fd_client, response, strlen(response));
				LOGD("Response : %s", response);
				close (fd_client);
				LOGI("Closing...");
				cJSON_Delete(json);
				exit (0);
			}
			else if (!strncmp (buf, "GET /wifi-scanner", 17))
			{
				LOGI("GET /wifi-scanner");
				char *response;
				bool _result_scanner;
				/* request wifi connection */
				std::string result_json = WifiRequest::Instance()->wifi_scanner(_result_scanner);
				if (_result_scanner) {
					response = HttpResponse::Instance()->create_response(
							(char*)HTTP_200_OK,
							(char*)result_json.c_str()
					);
				}
				else {
					response = HttpResponse::Instance()->create_response(
							(char*)HTTP_400_ERROR,
							(char*)WIFI_SCANNER_NOT_FOUND
					);
				}
				write(fd_client, response, strlen(response));
				LOGD("Response : %s", response);
				close (fd_client);
				LOGI("Closing...");
				exit(0);
			}
			else if (!strncmp (buf, "GET /metadata-request", 21))
			{
				LOGI("GET /metadata-request");
				/* request metadata */
				std::string result_json = MetaDataRequest::Instance()->metadata_request();
				char *response = HttpResponse::Instance()->create_response(
						(char*)HTTP_200_OK,
						(char*)result_json.c_str()
				);
				write(fd_client, response, strlen(response));
				LOGD("Response : %s", response);
				close (fd_client);
				LOGI("Closing...");
				exit(0);
			}
			else if (!strncmp (buf, "GET /ipaddress-request", 21))
			{
				LOGI("GET /ipaddress-request");
				/* request ipaddress */
				char *response;
				bool _result_ip;
				std::string result_json = MetaDataRequest::Instance()->ipaddress_request(_result_ip);
				if (_result_ip) {
					response = HttpResponse::Instance()->create_response(
							(char*)HTTP_200_OK,
							(char*)result_json.c_str()
					);
				} else {
					response = HttpResponse::Instance()->create_response(
							(char*)HTTP_400_ERROR,
							(char*)NO_WIFI_CONNECTION
					);
				}
				write(fd_client, response, strlen(response));
				LOGD("Response : %s", response);
				close (fd_client);
				LOGI("Closing...");
				exit(0);
			}
			else
			{
				LOGI("Wrong API");
				char *response = HttpResponse::Instance()->create_response(
					(char*)HTTP_405_ERROR,
					(char*)API_NOT_EXISTED
				);
				write(fd_client, response, strlen(response));
				LOGD("Response : %s", response);
				close (fd_client);
				LOGI("Closing...");
				exit(1);
			}	
		}
		close (fd_client);

		/* parent process */
	}

	return 0;
}
