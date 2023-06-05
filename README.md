# Q hardware

Qlocx has 5 different hardware types:

1. Q1 (BT)
2. Q2 (BT)
3. Q3-16 (BT)
4. Q3-32 (BT)
5. Qiot-1 (IOT)

Each folder in the project corresponds to a certain hardware type


## Remote SSH connection


On programming computer (host), run the following command:

`ngrok tcp 22`

The output should include a "Forwarding" value, e.g: `tcp://2.tcp.....:PORT -> localhost:22`


The client computer should run the following command:

`ssh qlocx@2.tcp... -p PORT` and should be prompted to enter password.

...and voila!


## Installing deps

After cloning the project, run `sudo chmod +x freshInstall.sh` to make installation script executable. Then, run `./freshInstall.sh` so that dependencies get installed. During installation, you will be prompted to enter your password. You might also be prompted to press Enter or answer Y/n to some questions. Always press Enter or answer Y.

Here is where we found the zip file that we are downloading through the sdk from nordicsemi:

https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs


CLI for above mentioned SDK: https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs

The JLink package has been manualle downloaded and placed in the repo due to the fact that it is not possible to download the .deb file through the command line (terms and conditions view shows up). Source: https://www.segger.com/downloads/jlink/JLink_Linux_V782b_x86_64.deb


## Running the project

It might be required to make `run.sh` executable. To do so, run `sudo chmod +x run.sh` in the project's root folder. Then, run `./run.sh <NAME_OF_FOLDER>`, e.g. `./run.sh q1`.
