## BMP280 ##

The BMP280 is an absolute barometric pressure sensor especially designed for 
mobile applications. 

## Inspecting BMP280 sensor using i2c-tools ##

Tools from `i2c-tools` package can be used for easy debugging of I2C devices 
from command line. Those tools reguire I2C kernel support which can be enabled 
from `raspi-config`. After selecting appropriate options and rebooting Raspberry 
Pi install `i2c-tools` with:

```
$ sudo apt-get install i2c-tools
```

Run `i2cdetect` tool to scan I2C bus for devices. Following command scans I2C 
bus `1`. 

```
$ i2cdetect -y 1
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

From results of the `i2cdetect` tool it is visible that some device exists at 
address `0x77`.  Program `i2cdump` can be used for reading registers of the I2C 
device at that address. 

```
$ i2cdump -y 1 0x77
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

Value `0x58` at address `0xD0` is interesting. From [BMP280 
documentation](https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf) 
we know that register at address `0xD0` contains chip id of value `0x58`. In 
other words, device at address `0x77` is indeed BMP280 sensor.  It is also 
visible that device is in sleep mode, because bits `0`, and `1` of register 
`0xF4` are not set. Execute following commands to put sensor in normal mode.

```
$ i2cset -y 1 0x77 0xF4 0x57
$ i2cset -y 1 0x77 0xF5 0x90
```

Sensor in normal mode periodically cycles between standby and measurement 
periods. Measurements are stored in registries from `0xF7` to `0xFC`. Subsequent 
runs of `i2cdump` show that values in those registries are changing over time. 
More information about BMP280 registries can be found in [BMP280 
documentation](https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-12.pdf).

## Installation ##

Make sure that directory of `arm-none-eabi-` toolchain is added to `PATH` 
environment variable, and that Raspbian Linux kernel source code is placed in 
`../../linux-raspi` directory relatively to `bmp280` directory. To compile 
driver run:

```
$ cd bmp280
$ make
```

Copy `bmp280.ko` file to Raspberry Pi.

Module `bmp280.ko` depends on `industrial.ko` and `kfifo_buf.ko` modules from 
`Industrial I/O` subsystem. For testing buffer functionality it is recommended 
to have `iio-trig-sysfs.ko` module ready. First step is to enable appropriate 
options in configuration of Raspbian Linux kernel source code.

```
$ cd ../../linux-raspi
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
            <M> SYSFS trigger
```

And compile modules.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Copy following files to Raspberry Pi.

```
drivers/iio/industrialio.ko
drivers/iio/kfifo_buf.ko
drivers/iio/trigger/iio-trig-sysfs.ko
```

I2C devices are not enumerated at the hardware level. For this reason Linux 
kernel must know device's address before loading a driver. There are several 
ways how to achieve this as described in 
[Documentation/i2c/instantiating-devices](https://github.com/raspberrypi/linux/blob/rpi-4.1.y/Documentation/i2c/instantiating-devices). 
For the sake of simplicity we are going to configure I2C device and address from 
user-space. Run following commands to inform Linux kernel that I2C device 
`bmp280` is on bus `1` and at address `0x77`.

```
$ su - -c "echo bmp280 0x77 > /sys/bus/i2c/devices/i2c-1/new_device"
```

Locate directory with kernel modules and execute.

```
$ sudo insmod industrial.ko
$ sudo insmod kfifo_buf.ko
$ sudo insmod bmp280.ko
```

If everything went ok then `dmesg | tail -4` command should return output like 
this:

```
$ dmesg | tail -4
[  433.650445] i2c i2c-1: new_device: Instantiated device bmp280 at 0x77
[  451.537025] bmp280 1-0077: Entering BMP280 probe...
[  451.537562] bmp280 1-0077: Chip id found: 0x58
[  451.541401] bmp280 1-0077: BMP280 registered.
```

String `BMP280 registered` indicates that BMP280 driver was able to connect to 
actual device, that it found correct chip id value, and that it loaded module 
into Linux kernel. 

## Usage ##

Driver exposes pressure and temperature measurements over virtual file system. 

```
$ cat /sys/bus/iio/devices/iio\:device0/in_pressure_input 
99534.601562500
$ cat /sys/bus/iio/devices/iio\:device0/in_temp_input 
23.490000000
```

These values mean that sensor measured pressure of 99534.601 Pa and temperature 
of 23.49 degrees Celsius. 

## IIO buffers and triggers ##

IIO makes it pretty easy to implement driver as a consumer of events that are 
generated by some trigger. Idea is that driver responds to events by writting 
data to corresponding memory ring buffer which is available to user space 
through character device in `/dev` sysfs. Following commands demonstrate 
a possible session.

First load `iio-trig-sysfs` module into kernel space.


```
$ sudo insmod iio-trig-sysfs.ko
```

Module creates `iio_sysfs_trigger` entry in `/sys/bus/iio/devices` directory. 

```
$ ls /sys/bus/iio/devices
iio:device0  iio_sysfs_trigger
```

To create an actual trigger execute following command.

```
$ su -
password:
# echo 1 > /sys/bus/iio/devices/iio_sysfs_trigger/add_trigger 
```

Result is a new entry `trigger0` in `/sys/bus/iio/devices` directory.

```
# ls /sys/bus/iio/devices
iio:device0  iio_sysfs_trigger  trigger0
```

To connect a trigger with the BMP280 driver write trigger's name to 
`current_trigger` pseudo file. 

```
# cat /sys/bus/iio/devices/trigger0/name
sysfstrig1
# echo sysfstrig1 > /sys/bus/iio/devices/iio:device0/trigger/current_trigger
```

Next step is to enable buffering on channel and driver level.

```
# echo 1 > /sys/bus/iio/devices/iio:device0/scan_elements/in_pressure_en
# echo 1 > /sys/bus/iio/devices/iio:device0/scan_elements/in_temp_en
# echo 1 > /sys/bus/iio/devices/iio:device0/buffer/enable
```

Create few events from trigger.

```
# echo 1 > /sys/bus/iio/devices/trigger0/trigger_now
# echo 1 > /sys/bus/iio/devices/trigger0/trigger_now
# echo 1 > /sys/bus/iio/devices/trigger0/trigger_now
```

Every event triggers driver to write 8 bytes of data to `/dev/iio:device0` file. 
Inspect data with `hexdump` program.

```
# sudo hexdump /dev/iio\:device0
0000000 08f7 0000 523a 0184 a18a 27d8 df64 1442
0000010 08f7 0000 52b9 0184 91d8 63b3 df64 1442
0000020 08f7 0000 530e 0184 3c82 a1ab df64 1442
```

First 2 bytes are temperature measurements, second 2 bytes pressure 
measurements, and the last 4 bytes define timestamp. Data is stored in 
little-endian format. In example above `0x000008f7` is `2295` which means 
`22.95` degrees Celsius. Value `0x0184523a` is `25449018` which means 
`99410.2265` Pa (`25449018 / 256`).

