## LSM9DS0 gyroscope, accelerometer, and magnetometer ##

The [LSM9DS0](http://www.st.com/web/catalog/sense_power/FM89/SC1448/PF258556) is 
a system-in-package featuring a 3D digital linear acceleration sensor, a 3D 
digital angular rate sensor, and a 3D digital magnetic sensor. 

## Installation ##

First make sure that directory of `arm-none-eabi-` toolchain is added to `PATH` 
environment variable. Second, place Raspbian Linux kernel source code in 
directory `../../linux-raspi` related to `lsm9ds0` directory. To compile driver 
run:

```
$ cd lsm9ds0
$ make
```

Copy `lsm9ds0.ko` file to Raspberry Pi.

Module `lsm9ds0.ko` depends on `industrial.ko` and `kfifo_buf.ko` modules from 
`Industrial I/O` subsystem. Those modules which should be compiled from Raspbian 
Linux kernel code.  Frist step is to enable them in kernel configuration.

```
$ cd linux-raspi
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
```

And compile `IIO` modules.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Copy following files to Raspberry Pi.

```
drivers/iio/industrialio.ko
drivers/iio/kfifo_buf.ko
```


## Usage ##

Connect to the Raspberry Pi and locate directory where kernel modules are 
uploaded.




