#!/bin/sh

killall udhcpd > /dev/null

dhd_priv iapsta_init mode gosta

# guess what wifi model we are using(light detect, may not match!!!)
INTERFACE=wlan1
DRIVER=nl80211

# delete default Gateway
# route del default gw 0.0.0.0 dev $INTERFACE
# release ip address
ifconfig $INTERFACE 0.0.0.0

ssid=$1
CHANNEL=$2

dhd_priv iapsta_config ifname  $INTERFACE ssid $ssid chan $CHANNEL amode open emode none
dhd_priv iapsta_enable ifname $INTERFACE
# start service
ifconfig $INTERFACE up

# set ip in ap mode
ifconfig $INTERFACE 10.10.10.254 netmask 255.255.255.0
# Add Gateway
# route add default gw 10.10.10.254
udhcpd /etc/udhcpd.conf

# mobile router support with LTE module
echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -t nat -A POSTROUTING -s 10.10.10.0/24 -j MASQUERADE && echo "iptable run"

exit 0
