# kernel-playground #
This repository contains experiments related to Linux kernel, Raspberry Pi 
platform, and QEMU. Focus is basically on ARM architecture. 

## Setup ##

* Used hardware:
  * [Raspberry Pi] (https://www.raspberrypi.org/)
  * [Adafruit Adafruit BMP280 I2C or SPI Barometric Pressure & Altitude Sensor] 
    (https://www.adafruit.com/products/2651)
* [Raspbian](https://www.raspberrypi.org/downloads/raspbian/) operating system
* arm-none-eabi-gcc cross compiler downloaded from 
  [https://launchpad.net/gcc-arm-embedded/](https://launchpad.net/gcc-arm-embedded/) 
* Linux kernel source code customized for Raspberry Pi cloned from 
  [https://github.com/raspberrypi/linux](https://github.com/raspberrypi/linux)


![Git log](http://mpod.github.io/img/log.png)


```
...
| | | * | | cd14299 BCM270X_DT: Adjust overlay README formatting
| | | |/ /  
| | | * | 0108373 Revert "bcm270x_dt: Use the sdhost MMC controller by default"
| | | * |   6330c27 (HEAD, tag: rpi-bootloader-1.20160202-1) Merge remote-tracking branch 'stable/linux-4.1.y' into rpi-4.1.y
| | | |\ \  
| | | | |/  
| | | | * 2d5f6b0 Linux 4.1.17
| | | | * d17367a recordmcount: Fix endianness handling bug for nop_mcount
...
```


