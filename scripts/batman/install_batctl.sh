#!/bin/bash

sudo apt-get install gcc git make
sudo apt install libnl-3-dev libnl-genl-3-dev
sudo su -
git clone https://git.open-mesh.org/batctl.git 
cd batctl 
sudo make install