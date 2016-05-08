# kernel-playground #

This repository contains experiments related to Linux kernel and Raspberry Pi.

## Contents ##

* [Driver for BMP280 barometric pressure sensor](https://github.com/mpod/kernel-playground/tree/master/bmp280)
    * [Inspecting BMP280 sensor using i2c-tools](https://github.com/mpod/kernel-playground/tree/master/bmp280#inspecting-bmp280-sensor-using-i2c-tools)
    * [Setting up development environment](https://github.com/mpod/kernel-playground/tree/master/bmp280#setting-up-development-environment)
    * [IIO buffers and triggers](https://github.com/mpod/kernel-playground/tree/master/bmp280#iio-buffers-and-triggers)
* [Driver for LSM9DS0 accelerometer, magnetometer, and gyroscope](https://github.com/mpod/kernel-playground/tree/master/lsm9ds0)
* [RTC triger for IIO subsystem](https://github.com/mpod/kernel-playground/tree/master/iio-trig-timer)
* [References](https://github.com/mpod/kernel-playground#references)

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
* Sensors
    * [BMP280](https://www.bosch-sensortec.com/bst/products/all_products/bmp280)
    * [LSM9DS0](http://www.st.com/web/catalog/sense_power/FM89/SC1448/PF258556#)
