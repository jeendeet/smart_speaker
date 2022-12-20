#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <signal.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>   

#define LOG_DISABLE     0
#define LOG_ERROR       1
#define LOG_WARNING     2
#define LOG_DEBUG       3
#define LOG_INFO        4
extern int log_level;

inline void setLogLevel(int level) {
    log_level = level;
}

/* Set log level */
int log_level = 5;
pid_t pid_avs, pid_con_wifi, pid_con_ble, pid_mosquitto;

#define LOGE(...) if(log_level > LOG_ERROR)     {printf(__VA_ARGS__); printf("\n");}
#define LOGW(...) if(log_level > LOG_WARNING)   {printf(__VA_ARGS__); printf("\n");}
#define LOGD(...) if(log_level > LOG_DEBUG)     {printf(__VA_ARGS__); printf("\n");}
#define LOGI(...) if(log_level > LOG_INFO)      {printf(__VA_ARGS__); printf("\n");}

/* execute command line in linux */
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try 
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) 
        {
            result += buffer;
        }
    } 
    catch (...) 
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

void pro_terminate(int sig){
    /* kill the child process */
    LOGI("\n AVS process killed %d\n", pid_avs);
    kill(pid_avs, SIGKILL);

    /* kill the conneted app child process */
    LOGI("\n Connected App process killed %d\n", pid_con_wifi);
    kill(pid_con_wifi, SIGKILL);

    /* kill the conneted app bluetooth child process */
    LOGI("\n Connected App Bluetooth process killed %d\n", pid_con_ble);
    kill(pid_con_ble, SIGKILL);

    /* kill the mosquitto child process */
    LOGI("\n Mosquitto process killed %d\n", pid_mosquitto);
    kill(pid_mosquitto, SIGKILL);

    exit(0);    
}

std::map<std::string, std::string> read_config (){
    std::map<std::string, std::string> mp; 
    std::ifstream cFile ("config.txt");
    if (cFile.is_open())
    {
        std::string line;
        while(getline(cFile, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
            if(line[0] == '#' || line.empty())
                continue;
            auto delimiterPos = line.find("=");
            std::string name = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            mp.insert({ name, value });
        }
        
    }
    else {
        std::cerr << "Couldn't open config file for reading.\n";
    }
    return mp;
}

void synchronize_clock(){
    std::map<std::string, std::string> config_syn; 
    config_syn = read_config ();

    std::string cmd = "cd " + config_syn["vSyn_clock_folder"] + \
        " && LD_LIBRARY_PATH=" + config_syn["ROOT_DIR"] + config_syn["vSyn_clock_library"] + \
        " " + config_syn["ROOT_DIR"] + config_syn["vSyn_clock_path"] + \
        " " + config_syn["vSyn_clock_server"] + \
        " " + ">> " + config_syn["ROOT_DIR"] + config_syn["vSyn_clock_log_file"] + " &";
    LOGI("exec cmd %s", cmd.c_str());
    std::string result = exec(cmd.c_str());
    LOGI("result %s", result.c_str());
}

void onboarding_wifi(){
    std::map<std::string, std::string> config_onbo_wifi; 
    config_onbo_wifi = read_config ();

    std::string cmd = "cd " + config_onbo_wifi["ROOT_DIR"] + config_onbo_wifi["vCon_wifi_folder"] + \
        " && LD_LIBRARY_PATH=" + config_onbo_wifi["ROOT_DIR"] + config_onbo_wifi["vCon_wifi_library"] + \
        " ./" + config_onbo_wifi["vCon_wifi_name"] + \
        " " + config_onbo_wifi["vCon_wifi_port"] + \
        " " + ">> " + config_onbo_wifi["ROOT_DIR"] + config_onbo_wifi["vCon_wifi_log_file"] + " &";
    LOGI("exec cmd %s", cmd.c_str());
    std::string result = exec(cmd.c_str());
    LOGI("result %s", result.c_str());
}

void onboarding_ble(){
    std::map<std::string, std::string> config_onbo_ble; 
    config_onbo_ble = read_config ();

    std::string cmd = "cd " + config_onbo_ble["vCon_ble_folder"] + \
        " && LD_LIBRARY_PATH=" + config_onbo_ble["ROOT_DIR"] + config_onbo_ble["vCon_ble_library"] + \
        " " + config_onbo_ble["ROOT_DIR"] + config_onbo_ble["vCon_ble_path"] + \
        " " + ">> " + config_onbo_ble["ROOT_DIR"] + config_onbo_ble["vCon_ble_log_file"] + " &";
    LOGI("exec cmd %s", cmd.c_str());
    std::string result = exec(cmd.c_str());
    LOGI("result %s", result.c_str());
}

void mosquitoo_pro(){
    std::map<std::string, std::string> config_mos; 
    config_mos = read_config ();

    std::string cmd = "cd " + config_mos["vMosquitto_folder"] + \
        " && LD_LIBRARY_PATH=" + config_mos["ROOT_DIR"] + config_mos["vMosquitto_library"] + \
        " " + config_mos["ROOT_DIR"] + config_mos["vMosquitto_path"] + \
        " -c " + config_mos["ROOT_DIR"] + config_mos["vMosquitto_file_config"] + \
        " " + ">> " + config_mos["ROOT_DIR"] + config_mos["vMosquitto_log_file"] + " &";
    LOGI("exec cmd %s", cmd.c_str());
    std::string result = exec(cmd.c_str());
    LOGI("result %s", result.c_str());
}

int main(){
    setLogLevel(log_level);
    std::string pid_avs_str, pid_con_wifi_str, pid_con_ble_str, pid_mosquitto_str;
    std::string cmd;
    std::string v_time;
    std::map<std::string, std::string> config; 
    config = read_config ();

    signal(SIGTERM, pro_terminate);
    /* synchronize clock process */
    if (config["vSyn_clock_register"] == "true")
    {
        synchronize_clock();
    }

    while(true) {

        /* mosquitto process */
        if (config["vMosquitto_register"] == "true")
        {
            cmd = "pidof -s " + config["vMosquitto_name"];
            pid_mosquitto_str = exec(cmd.c_str());

            if (pid_mosquitto_str.empty()) {
                sleep(2);
                mosquitoo_pro();
            }
            else
            {
                pid_mosquitto = (pid_t)stoi(pid_mosquitto_str);
            }
        }

        /* avs process */
        if (config["vAVS_register"] == "true")
        {
            cmd = "pidof -s " + config["vAVS_name"];
            pid_avs_str = exec(cmd.c_str());
            if (pid_avs_str.empty()) {
                sleep(3);
                cmd = "cd " + config["vAVS_folder"] + \
                    " && LD_LIBRARY_PATH=" + config["vAVS_library"] + \
                    " PA_ALSA_PLUGHW=" + config["vAVS_alsa_hw"] + \
                    " GST_PLUGIN_PATH=" + config["vAVS_gstreamer_plugin"] + \
                    " ./" + config["vAVS_name"] + \
                    " ./" + config["vAVS_config_file"] + \
                    " > /dev/null 2>&1 &";
                LOGI("exec cmd %s", cmd.c_str());
                std::string result = exec(cmd.c_str());
                LOGI("result %s", result.c_str());
            }
            else
            {
                pid_avs = (pid_t)stoi(pid_avs_str);
            }
        }

        /* onboarding wifi process */
        if (config["vOnbo_wifi_register"] == "true")
        {
            cmd = "pidof -s " + config["vCon_wifi_name"];
            pid_con_wifi_str = exec(cmd.c_str());
            if (pid_con_wifi_str.empty()) {
                sleep(2);
                onboarding_wifi();
            }
            else
            {
                pid_con_wifi = (pid_t)stoi(pid_con_wifi_str);
            }
        }

        /* onboarding bluetooth process */
        if (config["vOnbo_ble_register"] == "true")
        {
            cmd = "pidof -s " + config["vCon_ble_name"];
            pid_con_ble_str = exec(cmd.c_str());

            if (pid_con_ble_str.empty()) {
                sleep(2);
                onboarding_ble();
            }
            else
            {
                pid_con_ble = (pid_t)stoi(pid_con_ble_str);
            }
        }
        v_time = exec("date +%H:%M:%S");
        v_time = v_time.substr(0, v_time.size()-1);
        /* synchronize clock process */
        if ((config["vSyn_clock_register"] == "true") && 
            (v_time == config["vSyn_clock_schedule"]))
        {
            synchronize_clock();
        }

        // if (system("ping -c 1 8.8.8.8  > /dev/null 2>&1"))
        // {
        //     printf("No internet\n");

        // }
        // else
        //     printf("Internet\n");

    }
    return 0;
}
