# Overview

This project is a color editor application in HSV color model. Built and tested on PCA10059 by Nordic Semiconductor.

# Usage
To edit a color, you can physically double-click a button. On the first double-click you can change the hue component. To edit HSV component, long-press the button. To edit the next component, double-click again. After editing all three components, the color will be saved to the flash (persistent) memory of the device and the LEDs will display tuned color.

The second way to change the color is to use the nRF Connect application (or similiar) for Android and iOS using smartphone's BLE module.

<img src="https://github.com/masamonoke/ESTC-project/assets/68110536/2b16453b-a1a0-47af-a81e-bc70e0c1990c" width=200>
<img src="https://github.com/masamonoke/ESTC-project/assets/68110536/9da88f23-131c-4d1a-8734-45b8b56b153b" width=200>
<img src="https://github.com/masamonoke/ESTC-project/assets/68110536/34b1c1ce-5f6e-41c0-900a-dd3eb57d5ccb" width=200>
<img src="https://github.com/masamonoke/ESTC-project/assets/68110536/a28404b5-6a13-4028-988f-3bffab959586" width=200>
<img src="https://github.com/masamonoke/ESTC-project/assets/68110536/48101eb1-7995-4352-8ea7-8ddf2811d9b1" width=200>


# Build

To build you need Unix based OS like Linux or MacOS.
To build and flash you need to be in directory with Makefile and type into you terminal ```make dfu```.
For example, I use ```make dfu -j 4 SDK_ROOT=/Users/masamonoke/lib/esl-nsdk DFU_PORT=/dev/tty.usbmodemDA29A3A0D2851``` where ```SDK_ROOT``` is the location of your customized Nordic SDK https://github.com/DSRCorporation/esl-nsdk, ```DFU_PORT``` is what port to use to flash the board. By default ```DFU_PORT``` is ```/dev/ttyACM0``` that is Linux default and can be omitted when build on this OS.
Requirements for build is:
1.  nrfutil and Python 2.7 set as default Python version in system. You can get it by ```pip2 install nrfutil``` or ```pip install nrfutil```
2.  gcc-arm-none-eabi
3.  make
4.  build-essential
5.  picocom (to read logs). For example, I read logs with ```picocom /dev/tty.usbmodem0000000000001``` where ```/dev/tty.usbmodem0000000000001``` is what device will be seen in /dev after flash or ```/dev/ttyACM1``` on Linux
