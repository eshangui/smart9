#!/bin/bash

echo "hello world"

echo "shell name:$0";
echo "param1 = $1";
echo "param2 = $2";

#ifconfig wlan0 up
#wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf
#wpa_cli -i wlan0 add_network 

wpa_cli -i wlan0 set_network 0 ssid \"$1\"
sleep 5s
wpa_cli -i wlan0 set_network 0 psk \"$2\"
sleep 5s
wpa_cli -i wlan0 set_network 0 key_mgmt WPA-PSK
sleep 5s
wpa_cli -i wlan0 enable_network 0    
sleep 1s

wpa_cli -i wlan0 set update_config 1


wpa_cli -i wlan0 save_config

wpa_cli -i wlan0 list_network        
wpa_cli -i wlan0 select_network 0     
wpa_cli -i wlan0 enable_network 0     

sync

