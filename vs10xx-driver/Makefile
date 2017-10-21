
vs10xx-objs := vs10xx_iocomm.o vs10xx_queue.o vs10xx_device.o vs10xx_main.o

obj-m +=  vs10xx.o
PWD   := $(shell pwd)
all:
	@$(MAKE) ARCH=arm CROSS_COMPILE=${CCPREFIX} -C /home/r/linux M=$(PWD) modules
clean:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C /home/r/linux M=$(PWD) clean
