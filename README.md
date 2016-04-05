# kernel-playground #
This repository describes steps in creating a Linux kernel driver for [Adafruit 
BMP I2C or SPI Barometric Pressure & Altitude 
Sensor](https://www.adafruit.com/products/2651). Driver was tested on [Raspberry 
Pi](https://www.raspberrypi.org/) with 
[Raspbian](https://www.raspberrypi.org/downloads/raspbian/) OS installed. 
Development was done on desktop computer using 
[arm-none-eabi-gcc](https://launchpad.net/gcc-arm-embedded/) cross compiler 
toolchain.


## Inspecting BMP280 sensor from Raspberry Pi ##

To assemble BMP280 sensor follow directions from 
[here](https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout/).  
Sensor provides I2C and SPI interface, but for the purpose of this exercise only 
I2C is used. Information about I2C related pins on Raspberry Pi side can be 
found 
[here](http://elinux.org/RPi_Low-level_peripherals#General_Purpose_Input.2FOutput_.28GPIO.29).  

Tools from `i2c-tools` package can be used for easy debugging of I2C devices 
from command line. Those tools reguire I2C kernel support to be enabled in 
Raspberry Pi. So run `sudo raspi-config` and select appropriate options.  After 
rebooting Raspberry Pi install `i2c-tools` with following command.

```
pi@raspberrypi: sudo apt-get install i2c-tools
```

Run `i2cdetect` tool to scan I2C bus for devices. Following command scans I2C 
bus `1`.  

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

From results of the `i2cdetect` tool we can conclude that some device exists at 
address `0x77`. Program `i2cdump` can be used for reading registers of the I2C 
device at that address. 

```
pi@raspberrypi:~ $ i2cdump -y 1 0x77
No size specified (using byte-data access)
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef
00: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX    XXXXXXXXXXXXXXXX
10: XX 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    X...............
20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
30: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
80: 88 70 90 2a 1e 0f 72 00 51 6d c9 66 18 fc 31 94    ?p?*??r.Qm?f??1?
90: 88 d5 d0 0b 92 10 b3 00 f9 ff 8c 3c f8 c6 70 17    ???????.?.?<??p?
a0: 00 00 f8 00 00 00 00 00 00 00 00 00 33 00 00 c0    ..?.........3..?
b0: 00 54 00 00 00 00 60 02 00 01 ff f8 13 60 03 00    .T....`?.?.??`?.
c0: 00 00 00 ff 00 00 00 00 00 00 00 00 00 00 00 00    ................
d0: 58 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00    X?..............
e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................
f0: 00 00 00 00 00 00 00 80 00 00 80 00 00 00 00 00    .......?..?.....
```
Value `0x58` at address `0xD0` is interesting. In [BMP280 
documentation](https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf) 
is written that register at address `0xD0` contains chip id of value `0x58`. 
This confirms that device at address `0x77` is indeed BMP280 sensor. It is also 
visible that device is in sleep mode, because bits `0`, and `1` of register 
`0xF4` are not set. Execute following commands to put sensor in normal mode.

```
pi@raspberrypi:~ $ i2cset -y 1 0x77 0xF4 0x57
pi@raspberrypi:~ $ i2cset -y 1 0x77 0xF5 0x90
```

Sensor in normal mode periodically cycles between standby and measurement 
periods. Measurements are stored in registries from `0xF7` to `0xFC`. Subsequent 
runs of `i2cdump` show that values in those registries are changing over time.  
More information about BMP280 registries can be found in [BMP280 
documentation](https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf).


## Setting up Linux kernel source code ##

The Linux kernel header files should be available for building driver code. One 
way to achieve this is to download and use Linux kernel source code. Since the 
driver is going to be used on Rasperry Pi, it is recommended to use Raspbian 
version of Linux kernel. Clone Linux kernel source code. 

```
$ git clone https://github.com/raspberrypi/linux linux-raspi
```

It is recommended to clone kernel files to `linux-raspi` directory because 
driver's `Makefile` is set to search for header files in `../../linux-raspi`. 
Next step is to checkout kernel files to the same kernel version used on 
Raspberry Pi. Find kernel version on Raspberry Pi.

```
pi@raspberrypi:~ $ uname -a
Linux raspberrypi 4.1.17-v7+ #838 SMP Tue Feb 9 13:15:09 GMT 2016 armv7l GNU/Linux
```
In my case version `4.1.17` is used. To find right commit in git history run 
`git log` command and search for `4.1.17` string.

```
$ cd linux-raspi
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
From git log is visible that commit `6330c27` is a merge of Raspbian branch and 
Linux kernel version `4.1.17`. Checkout commit `6330c27`.

```
$ git checkout 6330c27
```

BMP280 driver depends on `Industrial I/0` (or IIO) support that it is not 
enabled by default in Raspbian OS. Configure kernel using following command.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- menuconfig
```

Select `Industrial I/O support` options.

```
Device Drivers --->
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

And compile `IIO` modules.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Copy following files to Raspberry Pi.

```
drivers/iio/industrialio.ko
drivers/iio/kfifo_buf.ko
drivers/iio/trigger/iio-trig-sysfs.ko
drivers/iio/trigger/iio-trig-interrupt.ko
```

## Linux driver for BMP280 sensor ##

Driver for BMP280 sensor is implemented in `bmp280/bmp280.c` file. 
Implementation is based on I2C and IIO support from Linux kernel. Run `make` to 
compile the driver source code. 

```
$ cd bmp280
$ make
```

Copy `bmp280/bmp280.ko` file to Raspberry Pi.

### Testing BMP280 driver ###

I2C devices are not enumerated at the hardware level. For this reason Linux 
kernel must know device's address before loading a driver. There are several 
ways how to achieve this as described in 
[Documentation/i2c/instantiating-devices](https://github.com/raspberrypi/linux/blob/rpi-4.1.y/Documentation/i2c/instantiating-devices).  
For the sake of simplicity we are going to configure I2C device and address from 
user-space. Run following commands to inform Linux kernel that I2C device 
`bmp280` is on bus `1` and at address `0x77`.

```
pi@raspberrypi:~ $ su -
Password: 
root@raspberrypi:~# echo bmp280 0x77 > /sys/bus/i2c/devices/i2c-1/new_device
```

Now we are ready to load driver files into kernel space. Files `industrialio.ko` 
and `kfifo_buf.ko` are prerequisite modules for loading `bmp280.ko` driver.

```
pi@raspberrypi: sudo insmode industrialio.ko
pi@raspberrypi: sudo insmode kfifo_buf.ko
pi@raspberrypi: sudo insmode bmp280.ko
```

Run following command to verify that kernel modules are loaded.

```
pi@raspberrypi:~/lkm $ lsmod
Module                  Size  Used by
bmp280                  3977  0 
kfifo_buf               2482  1 bmp280
industrialio           39851  2 bmp280,kfifo_buf
cfg80211              399468  0 
rfkill                 16799  2 cfg80211
r8712u                168059  0 
i2c_bcm2708             5062  0 
bcm2835_gpiomem         3023  0 
bcm2835_rng             1824  0 
evdev                  10552  0 
joydev                  9427  0 
snd_bcm2835            19739  0 
snd_pcm                75040  1 snd_bcm2835
snd_timer              19138  1 snd_pcm
snd                    52636  3 snd_bcm2835,snd_timer,snd_pcm
uio_pdrv_genirq         2986  0 
uio                     8228  1 uio_pdrv_genirq
i2c_dev                 6108  0 
fuse                   82598  1 
ipv6                  343556  30 

```

Following command displays last 10 lines from the kernel ring buffer. 

```
pi@raspberrypi:~ $ dmesg | tail
[    8.418592] cfg80211:   (5490000 KHz - 5730000 KHz @ 160000 KHz), (N/A, 2000 mBm), (0 s)
[    8.418607] cfg80211:   (5735000 KHz - 5835000 KHz @ 80000 KHz), (N/A, 2000 mBm), (N/A)
[    8.418622] cfg80211:   (57240000 KHz - 63720000 KHz @ 2160000 KHz), (N/A, 0 mBm), (N/A)
[   10.379810] smsc95xx 1-1.1:1.0 eth0: hardware isn't capable of remote wakeup
[   10.380311] IPv6: ADDRCONF(NETDEV_UP): eth0: link is not ready
[   19.188609] IPv6: ADDRCONF(NETDEV_CHANGE): wlan0: link becomes ready
[  433.650445] i2c i2c-1: new_device: Instantiated device bmp280 at 0x77
[  451.537025] bmp280 1-0077: Entering BMP280 probe...
[  451.537562] bmp280 1-0077: Chip id found: 0x58
[  451.541401] bmp280 1-0077: BMP280 registered.
```

String `BMP280 registered` indicates that BMP280 driver was able to connect to 
actual device, that it found correct chip id value, and that it loaded module 
into Linux kernel.

Driver exposes pressure and temperature measurements over virtual file system. 

```
pi@raspberrypi:~/lkm $ cat /sys/bus/iio/devices/iio\:device0/in_pressure_input 
99534.601562500
pi@raspberrypi:~/lkm $ cat /sys/bus/iio/devices/iio\:device0/in_temp_input 
23.490000000
```

These values mean that sensor measured pressure of 99534.601 Pa and temperature 
of 23.49 degrees Celsius. 

## References ##

* I2C
    * [I2C (Inter-Integrated Circuit) Bus Technical Overview and Frequently 
      Asked 
Questions](http://www.esacademy.com/en/library/technical-articles-and-documents/miscellaneous/i2c-bus.html)
    * [Documentation/i2c](https://github.com/raspberrypi/linux/tree/rpi-4.1.y/Documentation/i2c) 
      files in the kernel tree.
    * [Linux Kernel HTML Documentation, Chapter 11. I2C and SMBus 
      Subsystem](https://www.kernel.org/doc/htmldocs/device-drivers/i2c.html)
    * [Linux Journal, I2C Drivers, Part 
      I](http://www.linuxjournal.com/article/7136)
    * [Linux Journal, I2C Drivers, Part II](http://www.linuxjournal.com/article/7252)
* Industrial I/O
    * [Linux Kernel HTML Documentation, Industrial I/O driver developer's 
      guide](https://www.kernel.org/doc/htmldocs/iio/index.html)
    * [Analog Devices, Linux Industrial I/O 
      Subsystem](https://wiki.analog.com/software/linux/docs/iio/iio)
    * [IIO, a new kernel 
      subsystem](https://archive.fosdem.org/2012/schedule/event/693/127_iio-a-new-subsystem.pdf) 
    * [Industrial I/O Subsystem: The Home of Linux 
      Sensors](https://www.overleaf.com/articles/industrial-i-slash-o/dmqjqpzswtvb/viewer.pdf)
    * [drivers/stagging/iio/Documentation](https://github.com/raspberrypi/linux/tree/rpi-4.1.y/drivers/staging/iio/Documentation) 
      files in the kernel tree.
