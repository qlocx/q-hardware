#!/bin/bash

sudo apt-get install software-properties-common
sudo apt-add-repository universe
sudo apt-get update
sudo apt-get install -y python-pip
sudo apt install -y make

# the installation of node depends on the computer/debian version
# sudo apt install nodejs
# wget -qO- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.2/install.sh | bash
# source ~/.bashrc
# nvm install 16
# nvm use 16

sudo cp -r gcc-arm-none-eabi-7-2017-q4-major/* /usr/local

cd libsodium-1.0.17
export LDFLAGS='--specs=nosys.specs'
export CFLAGS='-Os -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard'
./configure --host=arm-none-eabi --prefix=/usr/local
make
sudo make install

cd ..

wget -O  nrf-command-line-tools_10.18.1_amd64.deb https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-18-1/nrf-command-line-tools_10.18.1_amd64.deb

sudo apt install ./JLink_Linux_V782b_x86_64.deb
sudo apt install ./nrf-command-line-tools_10.18.1_amd64.deb

sudo cp -r nrfjprog/* /usr/local/bin/
sudo apt install python3-pip
pip3 install nrfutil

npm i
# symlinks

ln -sTf `pwd`/node_modules `pwd`/q1/node_modules
ln -sTf `pwd`/node_modules `pwd`/q2/node_modules
ln -sTf `pwd`/node_modules `pwd`/q3_16/node_modules
ln -sTf `pwd`/node_modules `pwd`/q3_32/node_modules


ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q1/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q2/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q3_16/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q3_32/nRF5_SDK_15.0.0_a53641a


ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q1/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q2/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q3_16/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q3_32/gcc-arm-none-eabi-7-2017-q4-major

echo 'PATH=~/.local/bin:$PATH' >> ~/.bashrc
