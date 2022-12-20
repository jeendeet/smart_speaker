#/bin/bash wifi_connect_request.sh

killall wpa_supplicant
rm /opt/temp.conf
sleep 2

cp /etc/wpa_supplicant.conf /etc/wpa_supplicant.conf.bk
if [ -z "$2" ]
then
    echo "wifi no password"
    echo -e "network={\n     ssid=\"$1\"\n     key_mgmt=NONE\n}" >> /etc/wpa_supplicant.conf
else
    echo "wifi has password"
    echo -e "network={\n     ssid=\"$1\"\n     psk=\"$2\"\n}" >> /etc/wpa_supplicant.conf
fi

wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant.conf >> /opt/temp.conf &

a=0
check=0
while [ $a -lt 10 ]
do
    sleep 2
    echo $a
    a=`expr $a + 1`
    if grep -q "reason=WRONG_KEY" /opt/temp.conf; then
        killall wpa_supplicant
        echo "WRONG_KEY"
        mv /etc/wpa_supplicant.conf.bk /etc/wpa_supplicant.conf
        rm /opt/temp.conf
        a=0
        break
    elif grep -q "CTRL-EVENT-CONNECTED" /opt/temp.conf; then
        echo "CONNECTED"
        if ping -c 1 8.8.8.8; then
            echo "INTERNET_READY"
            check=1
            a=0
            exit 0
        else
            kill -9 $(ps -a | grep 'udhcpc -i wlan0 up' | awk '{print $1}')
            sleep 1
            udhcpc -i wlan0 up &
            sleep 3
            check=2
            a=0
            break
        fi
        
    fi
done
echo $check
echo $a
if [ $check -eq 2 ]; then
    if ping -c 1 8.8.8.8; then
        echo "INTERNET_READY_WITH_DHCP"
        exit 0
    else
        killall wpa_supplicant
        kill -9 $(ps -a | grep 'udhcpc -i wlan0 up' | awk '{print $1}')
        mv /etc/wpa_supplicant.conf.bk /etc/wpa_supplicant.conf
        rm /opt/temp.conf
        echo "NO_INTERNET"
        exit 1
    fi
fi

if [ $a -eq 10 ]; then
    killall wpa_supplicant
    echo "CANNOT_CONNECTED"
    mv /etc/wpa_supplicant.conf.bk /etc/wpa_supplicant.conf
    rm /opt/temp.conf
fi
