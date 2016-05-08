## BMP280 ##

The BMP280 is an absolute barometric pressure sensor especially designed for 
mobile applications. 

## Inspecting BMP280 sensor from user space ##

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

