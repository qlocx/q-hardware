#!/bin/bash

# Install Homebrew if not already installed
if ! command -v brew &> /dev/null; then
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install required packages
brew install python@2
brew install mpg123
brew install qrencode
brew install imagemagick
brew install node
brew install nvm

# Install GCC for ARM
brew install arm-none-eabi-gcc

# Install Ghostscript printer app via Homebrew Cask
brew install --cask ghostscript

# Install Node.js using nvm
nvm install 16
nvm use 16

# Install dependencies for compilation
brew install libsodium

# Download and install nrf-command-line-tools
curl -LO https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-18-1/nrf-command-line-tools_10.18.1_amd64.deb
sudo dpkg -i nrf-command-line-tools_10.18.1_amd64.deb

# Install JLink via Homebrew
brew install --cask segger-jlink

# Install pip for Python 3
brew install python@3
pip3 install nrfutil

# Install npm dependencies
npm install

# Create symlinks
ln -sTf "$(pwd)"/node_modules "$(pwd)"/q1/node_modules
ln -sTf "$(pwd)"/node_modules "$(pwd)"/q2/node_modules
ln -sTf "$(pwd)"/node_modules "$(pwd)"/q3_16/node_modules
ln -sTf "$(pwd)"/node_modules "$(pwd)"/q3_32/node_modules

ln -sTf "$(pwd)"/nRF5_SDK_15.0.0_a53641a "$(pwd)"/q1/nRF5_SDK_15.0.0_a53641a
ln -sTf "$(pwd)"/nRF5_SDK_15.0.0_a53641a "$(pwd)"/q2/nRF5_SDK_15.0.0_a53641a
ln -sTf "$(pwd)"/nRF5_SDK_15.0.0_a53641a "$(pwd)"/q3_16/nRF5_SDK_15.0.0_a53641a
ln -sTf "$(pwd)"/nRF5_SDK_15.0.0_a53641a "$(pwd)"/q3_32/nRF5_SDK_15.0.0_a53641a

ln -sTf "$(brew --prefix arm-none-eabi-gcc)/arm-none-eabi" "$(brew --prefix)"/q1/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf "$(brew --prefix arm-none-eabi-gcc)/arm-none-eabi" "$(brew --prefix)"/q2/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf "$(brew --prefix arm-none-eabi-gcc)/arm-none-eabi" "$(brew --prefix)"/q3_16/gcc-arm-none-eabi-7-2017-q4-major
ln -sTf "$(brew --prefix arm-none-eabi-gcc)/arm-none-eabi" "$(brew --prefix)"/q3_32/gcc-arm-none-eabi-7-2017-q4-major

# Add path to local binaries to .bash_profile
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bash_profile

# Source .bash_profile
source ~/.bash_profile
