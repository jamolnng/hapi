#!/bin/bash

wifi_name="hapinet"
wifi_pass="hapinetpass"

if [ "$EUID" -ne 0 ]; then
    echo "root permissions required to run!"
    exit
fi

# upgrade current software
sudo apt-get update
sudo apt-get upgrade -y

# setup wifi
sudo apt-get install dnsmasq hostapd dhcpcd5 -y
sudo systemctl stop dnsmasq
sudo systemctl stop hostapd

sudo sh -c 'echo "interface wlan0"                     >> /etc/dhcpcd.conf'
sudo sh -c 'echo "    static ip_address=1.1.1.1/24"    >> /etc/dhcpcd.conf'
sudo sh -c 'echo "    nohook wpa_supplicant"           >> /etc/dhcpcd.conf'
sudo service dhcpcd restart

sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
sudo sh -c 'echo "interface=wlan0"                                 >> /etc/dnsmasq.conf'
#sudo echo "usually wlan0"                                   >> /etc/dnsmasq.conf
sudo sh -c 'echo "dhcp-range=1.1.1.2,1.1.1.20,255.255.255.0,24h" >> /etc/dnsmasq.conf'

sudo sh -c 'echo "interface=wlan0"             >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "driver=nl80211"              >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "ssid=$wifi_name"             >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "hw_mode=g"                   >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "channel=7"                   >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "wmm_enabled=0"               >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "macaddr_acl=0"               >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "auth_algs=1"                 >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "ignore_broadcast_ssid=0"     >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "wpa=2"                       >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "wpa_passphrase=$wifi_pass"   >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "wpa_key_mgmt=WPA-PSK"        >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "wpa_pairwise=TKIP"           >> /etc/hostapd/hostapd.conf'
sudo sh -c 'echo "rsn_pairwise=CCMP"           >> /etc/hostapd/hostapd.conf'

sudo sh -c 'echo "DAEMON_CONF=\"/etc/hostapd/hostapd.conf\"" >> /etc/default/hostapd'

sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd
sudo systemctl start dnsmasq

# install build tools
sudo apt-get install build-essential -y

# install spinnaker sdk
sudo apt-get install libraw1394-11 libusb-1.0-0 -y
tar xvfz spinnaker-1.20.0.14-armhf-pkg.tar.gz
cd spinnaker-1.20.0.14-armhf/
sudo sh install_spinnaker_arm.sh
cd ..
# sudo echo "usbcore.usbfs_memory_mb=1000" >> /boot/extlinux/extlinux.conf

# install git
sudo apt-get install git -y

# install wiringPi
git clone git://git.drogon.net/wiringPi
cd wiringPi
sudo ./build
cd ..

# install cmake
sudo apt-get install cmake -y

### UBUNTU ###
sudo add-apt-repository ppa:ubuntu-raspi2/ppa
sudo apt-get update
sudo groupadd -f --system gpio
sudo groupadd -f --system i2c
sudo groupadd -f --system input
sudo groupadd -f --system spi
sudo add-apt-repository ppa:ubuntu-pi-flavour-makers/ppa
sudo apt-get update
sudo apt-get install wiringpi libwiringpi2 libwiringpi2-dev
##############

# install imagemagick
sudo apt-get install imagemagick -y

# configure usb
sudo sh -c "echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb"

# build and install hapi programs
cd ../../
mkdir build
cd build
cmake ..
make
sudo make install

### web portal ###

# shell in a box
sudo apt-get install libssl-dev libpam0g-dev zlib1g-dev dh-autoreconf shellinabox -y

# create service
cd ../tools/setup
sudo cp hapiweb /etc/init.d/hapiweb
sudo chmod 755 /etc/init.d/hapiweb
sudo update-rc.d hapiweb defaults
sudo rm /etc/init.d/shellinabox

# configure hapi
sudo /usr/local/bin/hapi-config

# refresh
echo "Rebooting in 5 seconds..."
sleep 5
sudo reboot
