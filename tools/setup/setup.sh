#!/bin/bash

# install spinnaker sdk
sudo apt-get install libraw1394-11 libusb-1.0-0 -y
tar xvfz spinnaker-1.15.0.63-armhf-pkg.tar.gz
cd spinnaker-1.15.0.63-armhf/
sudo sh install_spinnaker_arm.sh
cd ..

# install git
sudo sudo apt-get install git -y

# install wiringPi
git clone git://git.drogon.net/wiringPi
cd wiringPi
sudo ./build

# install cmake
sudo apt-get install cmake -y