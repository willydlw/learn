#!/bin/bash

if [ -f /etc/debian_version ]; then
    echo "file /etc/debian_version exists"
    command='sudo service network-manager stop'
    echo "running command: $command"
    sudo service network-manager stop
elif [ -f /etc/arch-release ]; then
    echo "file /etc/arch-release exists"
    command='sudo pkill NetworkManager'
    echo "running command: $command"
    sudo pkill NetworkManager
else
    echo "unknown file system"
    exit 
fi

# load batman-adv module 
echo "loading batman-adv kernel module"
modprobe --verbose batman-adv

# wait for interface to be released by network manager
# in case a resource busy error message popped up
echo "sleeping for 2 seconds"
sleep 2

# Your ethernet and wireless interface names may
# not be eth0, wlan0. Run ifconfig to find your
# interface names and replace etho, wlan0 with them
# before running the script
command = 'ip link set up dev eth0'
echo -e "\nrunning command: $command"
ip link set up dev eth0

# batman requires mtus of 1532
command = 'ip link set mtu 1532 dev wlan0'
echo -e "\nrunning command: $command"
ip link set mtu 1532 dev wlan0

# add interface to the ad-hoc network
echo -e "\nchanging to ad-hoc mode"
command='iwconfig wlan0 mode ad-hoc channel 8 essid MARBLE ap 02:12:34:56:78:9A'
echo "running command: $command"



# add wlan0 to batman-adv virtual interface to enable c
# communication with other batman-adv nodes
echo -e "\nadding wireless interface to batman"
command='batctl if add wlan0'
echo "running command $command"


# bring up wireless interface
echo -e "\nbring up wireless interface"
command='ifconfig wlan0 up'
echo "running command $command"


# bring up bat0 interface
echo "bring up bat0 interface"
command="ifconfig bat0 192.168.1.4 up"
echo "running command $command"

echo -e "\nscript finished"
