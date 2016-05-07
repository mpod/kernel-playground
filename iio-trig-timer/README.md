## RTC triger for IIO subsystem ##

This kernel module adds a `IIO` trigger that creates events at defined 
frequency. It depends on `RTC` device under `/dev/rtc0` name. `IIO` drivers can 
use this trigger to write measurements into corresponding ring buffer at certain 
frequency. 

## Installation ##

First make sure that directory of `arm-none-eabi-` toolchain is added to `PATH` 
environment variable. Second, place Raspbian Linux kernel source code in 
`../../linux-raspi` directory relatively to `iio-trig-timer` directory. To 
compile driver run:

```
$ cd iio-trig-timer
$ make
```

Copy `iio-trig-timer.ko` file to Raspberry Pi.

Module `iio-trig-timer.ko` depends on `industrial.ko` module from `Industrial 
I/O` subsystem. It also depends on `/dev/rtc0` real time clock device. Since 
Raspberry Pi doesn't have real time clock chip it is possible to use 
`rtc-test.ko` module as an alternative. First step is to enable appropriate 
options in Raspbian Linux kernel source code.

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
    [*] Real Time Clock --->
        <M> Test driver/device
```

And compile modules.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Copy following files to Raspberry Pi.

```
drivers/iio/industrialio.ko
drivers/rtc/rtc-test.ko
```

Connect to the Raspberry Pi. Locate directory with kernel modules and execute.

```
$ sudo insmod industrial.ko
$ sudo insmod rtc-test.ko
$ sudo insmod iio-trig-timer.ko
```

If everything went ok then `dmesg | tail -1` command should return output like 
this:

```
$ dmesg | tail -1
[  102.992538] iio trigger0: Registered successfully.
```

## Usage ##


