# kernel-playground #
This repository contains experiments related to Linux kernel, Raspberry Pi 
platform, and QEMU. Focus is basically on ARM architecture. 

* Used hardware:
  * [Raspberry Pi](https://www.raspberrypi.org/)
  * [Adafruit Adafruit BMP280 I2C or SPI Barometric Pressure & Altitude Sensor](https://www.adafruit.com/products/2651)
* [Raspbian](https://www.raspberrypi.org/downloads/raspbian/) operating system
* arm-none-eabi-gcc cross compiler downloaded from 
  [https://launchpad.net/gcc-arm-embedded/](https://launchpad.net/gcc-arm-embedded/) 
* Linux kernel source code customized for Raspberry Pi from 
  [https://github.com/raspberrypi/linux](https://github.com/raspberrypi/linux)

### Linux kernel setup ###

Clone Linux Raspberry Pi repository.
```
$ git clone https://github.com/raspberrypi/linux
```
Find out kernel version used in Raspbian OS.
```
pi@raspberrypi:~ $ uname -a
Linux raspberrypi 4.1.17-v7+ #838 SMP Tue Feb 9 13:15:09 GMT 2016 armv7l GNU/Linux
```
Search in git log for commit related to kernel version 4.1.17.
```
$ git log --oneline --decorate --graph --all
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
Checkout commit 6330c27.
```
$ git checkout 6330c27
```
Configure kernel.
```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- menuconfig
```
Examples from this repository require support for I2C and Industrial I/O. Select 
appropriate options in kernel configuration.

```
Device Drivers --->
    I2C support --->
        <M> I2C device interface
    <M> Industrial I/O support --->
        -*- Enable buffer supper within IIO
            [*] IIO callback buffer used for push in-kernel interface
            -M- Industrial I/O buffering based on kfifo
        -*- Enable triggered sampling support
            (2) Maximum number of consumers per trigger
        Triggers - standalone --->
            <M> Generic interrupt trigger
            <M> SYSFS trigger
```

Compile modules.
```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Generated files are:
```
drivers/i2c/i2c-dev.ko
drivers/iio/industrialio.ko
drivers/iio/kfifo_buf.ko
drivers/iio/trigger/iio-trig-sysfs.ko
drivers/iio/trigger/iio-trig-interrupt.ko

```

Copy them to Raspberry Pi.
