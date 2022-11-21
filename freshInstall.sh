sudo apt-get install software-properties-common
sudo apt-add-repository universe
sudo apt-get update
sudo apt-get install -y python-pip
sudo apt install -y make
sudo apt install -y nodejs
sudo apt install -y npm

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
ln -sTf `pwd`/node_modules `pwd`/q3_16ports/node_modules
ln -sTf `pwd`/node_modules `pwd`/q3_32ports/node_modules


ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q1/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q2/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q3_16ports/nRF5_SDK_15.0.0_a53641a
ln -sTf `pwd`/nRF5_SDK_15.0.0_a53641a `pwd`/q3_32ports/nRF5_SDK_15.0.0_a53641a


ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q1/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q2/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q3_16ports/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf `pwd`/gcc-arm-none-eabi-7-2017-q4-major `pwd`/q3_32ports/gcc-arm-none-eabi-7-2017-q4-major

echo 'PATH=~/.local/bin:$PATH' >> ~/.bashrc
