# kernel-playground #
This repository contains my experiments related to Linux kernel, Raspberry Pi, 
and QEMU. 

## Setup ##
Focus of this project is Raspberry Pi platform, or in other words ARM 
architecture. 

### Raspbian ###
To make Raspberry Pi functional it is recommended to download and install 
[https://www.raspberrypi.org/downloads/raspbian/](Raspbian) operating system.

### Toolchain ###
One way is to download and extract ARM toolchain package from 
[https://launchpad.net/gcc-arm-embedded/](https://launchpad.net/gcc-arm-embedded/).  
It is useful to add bin directory of extracted package to PATH environment 
variable. Another way is just to install arm-none-eabi-gcc package from Linux 
distribution repository.
