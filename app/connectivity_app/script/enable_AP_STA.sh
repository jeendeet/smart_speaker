#/bin/bash enanble_AP_STA.sh

sudo iw dev wlan0 interface add wlan1 type __ap
sleep 2
sudo ifconfig wlan1 192.168.50.5
sleep 1
sudo wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf -B &
sleep 2
wpa_cli -p /var/run/wpa_supplicant -i wlan0 enable_network 0
sleep 2

sudo systemctl stop hostapd
sudo systemctl start hostapd

sudo systemctl stop dnsmasq
sudo systemctl start dnsmasq
