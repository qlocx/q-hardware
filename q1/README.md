# dfu

### todo:
* use local settings for *nRF5_SDK_15.0.0_a53641a/modules/nrfx/mdk/nrf_common.ld* link script

### installing dependencies for building

```bash
apt install -y make
wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2017q4/gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
tar -xf gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
cp -r gcc-arm-none-eabi-7-2017-q4-major/* /usr/local
cd /tmp
wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.16.tar.gz
tar -xvzf libsodium-1.0.16.tar.gz
cd libsodium-1.0.16
export LDFLAGS='--specs=nosys.specs'
export CFLAGS='-Os -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard'
./configure --host=arm-none-eabi --prefix=/usr/local
make
make install
cd /tmp
# wget https://www.nordicsemi.com/eng/nordic/download_resource/51388/27/62239128/94917
wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-9-0/nRFCommandLineTools1090Linuxamd64tar.gz
tar -xf 94917
cp -r nrfjprog/* /usr/local/bin/
apt install python-pip
pip install nrfutil
echo 'PATH=~/.local/bin:$PATH' >> ~/.bashrc

```

### memory layout

* `0x00000 - 0x03000` mbr (12 kb)
* `0x03000 - 0x26000` softdevice (140 kb)
* `0x26000 - 0x6d000` application (284 kb)
* `0x6d000 - 0x6f000` fstorage flash (8 kb)
* `0x6f000 - 0x72000` fds flash (12 kb)
* `0x72000 - 0x7f000` bootloader (52 kb)
* `0x7f000 - 0x80000` bootloader settings (4 kb)

### board versions

#### q1a

#### q1b

#### q1c

#### q2

### CHANGELOG

#### version 2

Changed the order and maximum length of the fields in `qlocx_api_open_port`.

### version 3

Reduced current consumption by `0.20 mA` for `12V` by disabling current measurement when not in use.


### notes

Sense: Open port 1 for 1 sec after triggering

030a010a00000000000000000000

03(command)

0a(triggering-port)

01(out-port)

0a(duration)

00(delay)-repeat 000000000000000000


