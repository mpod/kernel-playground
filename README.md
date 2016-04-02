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

## Linux kernel setup ##

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


## Inspecting BMP280 with i2c-tools ##

First assemble and connect BMP280 and Raspberry Pi following directions from 
[here] 
(https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout/).  
To test I2C device it is recommented to use i2c-tools. Install them with 
following command:

```
pi@raspberrypi: sudo apt-get install i2c-tools
```

Run i2cdetect tool to scan I2C bus for devices. Following command scans I2C bus 
1 and it finds BMP280 device at address 0x77.

```
pi@raspberrypi:~ $ i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- 77
```
Tool i2cdump outputs contents of device's registers.
```
pi@raspberrypi:~ $ i2cdump -y 1 0x77
No size specified (using byte-data access)
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
10: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
30: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
80: 88 70 90 2a 1e 0f 72 00 51 6d c9 66 18 fc 31 94    ?p?*??r.Qm?f??1?
90: 88 d5 d0 0b 92 10 b3 00 f9 ff 8c 3c f8 c6 70 17    ???????.?.?<??p?
a0: 00 00 f8 00 00 00 00 00 00 00 00 00 33 00 00 c0    ..?.........3..?
b0: 00 54 00 00 00 00 60 02 00 01 ff f8 13 71 03 00    .T....`?.?.??q?.
c0: 08 00 57 ff 00 00 00 00 00 00 00 00 00 00 00 00    ?.W.............
d0: 58 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00    X?..............
e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
f0: 00 00 00 00 57 90 00 5b 95 40 7f 2b 00 00 00 00    ....W?.[?@?+....
```
From (BMP280 documentation)
[https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf] 
it is known that register at address 0xD0 contains chip id of value 0x58, which 
is actually visible in output of i2cdump command.

Register value can be set with i2cset command. Check out in BMP280 documentation 
how to put device in measurement mode.
