obj-m+=lsm9ds0-gyro.o
export ARCH:=arm
export CROSS_COMPILE:=arm-none-eabi-

all:
		make -C ../../linux-raspi M=$(PWD) modules
clean:
		make -C ../../linux-raspi M=$(PWD) clean
