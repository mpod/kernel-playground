# kernel-playground #
This repository contains experiments related to Linux kernel, Raspberry Pi 
platform, and QEMU. Focus is basically on ARM architecture. 

## Setup ##

### Hardware ###
Code from this repository is implemented for following hardware:
* [Raspberry Pi] (https://www.raspberrypi.org/)
* [Adafruit Adafruit BMP280 I2C or SPI Barometric Pressure & Altitude Sensor] 
  (https://www.adafruit.com/products/2651)

### Raspbian ###
It is recommended to download and install
[Raspbian](https://www.raspberrypi.org/downloads/raspbian/) operating system on 
Raspberry Pi. Examples from the repository are tested on Raspbian OS.


### Toolchain ###
One way is to download and extract ARM toolchain package from 
[https://launchpad.net/gcc-arm-embedded/](https://launchpad.net/gcc-arm-embedded/).  
It is useful to add bin directory of extracted package to PATH environment 
variable. Another way is just to install arm-none-eabi-gcc package from Linux 
distribution repository.

## Linux kernel ##

    $ git clone https://github.com/raspberrypi/linux
