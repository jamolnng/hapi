#!/bin/bash

wifi_name = "hapinet"
wifi_pass = "hapinetpass"

if [ "$EUID" -ne 0 ]; then
    echo "root permissions required to run!"
    exit
fi

# upgrade current software
sudo apt-get update
sudo apt-get upgrade -y

# setup wifi
sudo apt-get install dnsmasq hostapd
sudo systemctl stop dnsmasq
sudo systemctl stop hostapd

sudo echo "interface wlan0"                     >> /etc/dhcpcd.conf
sudo echo "    static ip_address=1.1.1.1/24"    >> /etc/dhcpcd.conf
sudo echo "    nohook wpa_supplicant"           >> /etc/dhcpcd.conf
sudo service dhcpcd restart

sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
sudo echo "interface=wlan0"                                 >> /etc/dnsmasq.conf
sudo echo "usually wlan0"                                   >> /etc/dnsmasq.conf
sudo echo "  dhcp-range=1.1.1.2,1.1.1.20,255.255.255.0,24h" >> /etc/dnsmasq.conf

sudo echo "interface=wlan0"             >> /etc/hostapd/hostapd.conf
sudo echo "driver=nl80211"              >> /etc/hostapd/hostapd.conf
sudo echo "ssid=$wifi_name"             >> /etc/hostapd/hostapd.conf
sudo echo "hw_mode=g"                   >> /etc/hostapd/hostapd.conf
sudo echo "channel=7"                   >> /etc/hostapd/hostapd.conf
sudo echo "wmm_enabled=0"               >> /etc/hostapd/hostapd.conf
sudo echo "macaddr_acl=0"               >> /etc/hostapd/hostapd.conf
sudo echo "auth_algs=1"                 >> /etc/hostapd/hostapd.conf
sudo echo "ignore_broadcast_ssid=0"     >> /etc/hostapd/hostapd.conf
sudo echo "wpa=2"                       >> /etc/hostapd/hostapd.conf
sudo echo "wpa_passphrase=$wifi_pass"   >> /etc/hostapd/hostapd.conf
sudo echo "wpa_key_mgmt=WPA-PSK"        >> /etc/hostapd/hostapd.conf
sudo echo "wpa_pairwise=TKIP"           >> /etc/hostapd/hostapd.conf
sudo echo "rsn_pairwise=CCMP"           >> /etc/hostapd/hostapd.conf

sudo echo >> "DAEMON_CONF=\"/etc/hostapd/hostapd.conf\"" >> /etc/default/hostapd

sudo systemctl start hostapd
sudo systemctl start dnsmasq

# install build tools
sudo apt-get install build-essential -y

# install spinnaker sdk
sudo apt-get install libraw1394-11 libusb-1.0-0 -y
tar xvfz spinnaker-1.15.0.63-armhf-pkg.tar.gz
cd spinnaker-1.15.0.63-armhf/
sudo sh install_spinnaker_arm.sh
cd ..
sudo echo "usbcore.usbfs_memory_mb=1000" >> /boot/extlinux/extlinux.conf

# install git
sudo apt-get install git -y

# install wiringPi
git clone git://git.drogon.net/wiringPi
cd wiringPi
sudo ./build
cd ..

# install cmake
sudo apt-get install cmake -y

# configure usb
sudo echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb

# build and install hapi programs
cd ../../
cmake .
make
sudo make install

# create service
sudo echo "Description=HAPI service"                    >> /etc/systemd/system/hapi.service
sudo echo ""                                            >> /etc/systemd/system/hapi.service
sudo echo "Wants=network.target"                        >> /etc/systemd/system/hapi.service
sudo echo "After=syslog.target network-online.target"   >> /etc/systemd/system/hapi.service
sudo echo ""                                            >> /etc/systemd/system/hapi.service
sudo echo "[Service]"                                   >> /etc/systemd/system/hapi.service
sudo echo "Type=simple"                                 >> /etc/systemd/system/hapi.service
sudo echo "ExecStart=/usr/local/bin/hapi"               >> /etc/systemd/system/hapi.service
sudo echo "Restart=on-failure"                          >> /etc/systemd/system/hapi.service
sudo echo "RestartSec=10"                               >> /etc/systemd/system/hapi.service
sudo echo "KillMode=process"                            >> /etc/systemd/system/hapi.service
sudo echo ""                                            >> /etc/systemd/system/hapi.service
sudo echo "[Install]"                                   >> /etc/systemd/system/hapi.service
sudo echo "WantedBy=multi-user.target"                  >> /etc/systemd/system/hapi.service

sudo systemctl daemon-reload
sudo systemctl enable hapi

# configure hapi
sudo /usr/local/bin/hapi-config

# refresh
echo "Rebooting in 5 seconds..."
sleep 5
sudo reboot
