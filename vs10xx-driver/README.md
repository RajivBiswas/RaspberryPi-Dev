# Writing a Linux Device Driver for Audio DSP Codec, VS1053 for RaspberryPi 3 on Linux Kernel 4.4.

![Alt text](https://github.com/RajivBiswas/RaspberryPi-Dev/blob/master/small_setup.png "The setup")

Linux Device Driver Sources for VS1053/1003 Audio Codec.

To Build the Project first setup the Toolchain as i have mentioned in my Blog Page:
```
https://blogsmayan.blogspot.in/p/linux-device-driver.html
```
Please refer the section, "Building the Driver, Toolchain setup, Device Tree & Makefile".

Please ensure to change the Makefile, the Path where you extracted your Linux Kernel source.\
Mine was at /home/r/linux
```
vs10xx-objs := vs10xx_iocomm.o vs10xx_queue.o vs10xx_device.o vs10xx_main.o

obj-m +=  vs10xx.o
PWD   := $(shell pwd)
all:
	@$(MAKE) ARCH=arm CROSS_COMPILE=${CCPREFIX} -C /home/r/linux M=$(PWD) modules
clean:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C /home/r/linux M=$(PWD) clean
```
After Building the Kernel sources, modules & DTs successfully, go inside the sources\
of this Linux Driver & type:
```
make clean
make all
```
After successful build, copy the entire folder to your Rpi3 Home path using SD Card Reader.\
After this power on your Rpi3 & go to the directory path of this Linux Device Driver.\
Insert the Module using insmod
```
insmod vs10xx.ko
```
Check the dmesg logs
```
dmesg
```
The dmesg logs should show correct VERSION string for your Audio Codec, be it VS1003/VS1053/VS1063\
et al.\
Check in \dev directory. There must be an entry by \dev\vs10xx-0

Regards,\
Rajiv.
