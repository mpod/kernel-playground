## LSM9DS0 gyroscope, accelerometer, and magnetometer ##

The [LSM9DS0](http://www.st.com/web/catalog/sense_power/FM89/SC1448/PF258556) is 
a system-in-package featuring a 3D digital linear acceleration sensor, a 3D 
digital angular rate sensor, and a 3D digital magnetic sensor. 

## Instalation ##

First make sure that `arm-none-eabi-` toolchain is available and that Raspbian 
Linux kernel source code is placed in `../../linux-raspi` direcory relatively to 
`lsm9ds0` directory. 

```
$ cd lsm9ds0
$ make
```

Copy `lsm9ds0.ko` file to Raspberry Pi.




