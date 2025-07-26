obj-m := hello_world.o

KDIR := /lib/modules/4.19.94/build
PWD := $(shell pwd)
ARCH=arm 
CROSS_COMPILE=arm-linux-gnueabihf-
HOST_KDIR := /lib/modules/$(shell uname -r)/build

exe:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

info:
	modinfo hello_world.ko

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean

modules_install:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules_install

host:
	make -C $(HOST_KDIR) M=$(PWD) modules

send:
	scp -r ../0_hello_world debian@192.168.7.2:/home/debian/workspace/device_drivers


help:
	@echo "Usage:"
	@echo "  make modules         - Build the module"
	@echo "  make modules_install - Install the module"
	@echo "  make clean          - Clean the build files"
	@echo "  make help           - Show this help message"
	#make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help






