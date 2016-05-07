## LSM9DS0 gyroscope, accelerometer, and magnetometer ##

The [LSM9DS0](http://www.st.com/web/catalog/sense_power/FM89/SC1448/PF258556) is 
a system-in-package featuring a 3D digital linear acceleration sensor, a 3D 
digital angular rate sensor, and a 3D digital magnetic sensor. 

## Installation ##

First make sure that directory of `arm-none-eabi-` toolchain is added to `PATH` 
environment variable. Second, place Raspbian Linux kernel source code in 
`../../linux-raspi` directory relatively to `lsm9ds0` directory. To compile 
driver run:

```
$ cd lsm9ds0
$ make
```

Copy `lsm9ds0.ko` file to Raspberry Pi.

Module `lsm9ds0.ko` depends on `industrial.ko` and `kfifo_buf.ko` modules from 
`Industrial I/O` subsystem. First step is to enable appropriate options in 
Raspbian Linux kernel source code.

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
```

And compile modules.

```
$ make -j 4 -k ARCH=arm CROSS_COMPILE=arm-none-eabi- modules
```

Copy following files to Raspberry Pi.

```
drivers/iio/industrialio.ko
drivers/iio/kfifo_buf.ko
```

Connect to the Raspberry Pi. Linux kernel should be first informed that 
`LSM9DS0` chip is connected to I2C bus. 

```
$ su - -c "echo lsm9ds0_accel_magn 0x1d > /sys/bus/i2c/devices/i2c-1/new_devicew_device"
$ su - -c "echo lsm9ds0_gyro 0x6b > /sys/bus/i2c/devices/i2c-1/new_device"
```

Locate directory with kernel modules and execute.

```
$ sudo insmod industrial.ko
$ sudo insmod kfifo_buf.ko
$ sudo insmod lsm9ds0.ko
```

If everything went ok then `dmesg | tail -4` command should return output like 
this:

```
$ dmesg | tail -4
[168008.825324] i2c i2c-1: new_device: Instantiated device lsm9ds0_gyro at 0x6b
[168030.604269] i2c i2c-1: new_device: Instantiated device lsm9ds0_accel_magn at 0x1d
[168058.096510] lsm9ds0 1-006b: Gyroscope found.
[168058.100451] lsm9ds0 1-001d: Accelerometer and magnetometer found.
```

## Usage ##

`LSM9DS0` driver creates two directories under `/sys/bus/iio/devices/` 
directory. 

```
$ ls /sys/bus/iio/devices
iio:device0  iio:device1
```
One directory implements gyroscope interface, while other directory implements 
accelerometer and magnetometer interface. To get the latest measurements from 
gyroscope sensor read `in_anglvel_{x|y|z}_raw}` files, for accelerometer read 
`in_accel_{x|y|z}_raw`, while for magnetometer read `in_magn_{x|y|z}_raw` files.

