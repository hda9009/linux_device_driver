obj-m := my_platform_driver.o

PROJECT := 6_dt_platform_device
KDIR := /lib/modules/5.10.168/build
PWD := $(shell pwd)
ARCH=arm 
CROSS_COMPILE=arm-linux-gnueabihf-
HOST_KDIR := /lib/modules/6.14.0-24-generic/build


exe:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

info:
	modinfo main.ko

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean

modules_install:
	sudo make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules_install

host:
	@echo "kernel directory: $(HOST_KDIR)"
	make -C $(HOST_KDIR) M=$(PWD) modules

send:
	scp -r $(PATH) debian@192.168.7.2:/home/debian/workspace/device_drivers

dtb:
	scp -r /home/hda/Desktop/workspace/linux-5.10.168-ti-r83/arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.7.2:/home/debian/workspace/device_drivers

dtb_send:
	sudo mount /dev/mmcblk0p3 /mnt
	sudo scp -r am335x-boneblack.dtb /mnt/boot/dtbs/5.10.168-ti-r83/
	
r:
	@echo "Executing the module..."
	sudo insmod main.ko
	sudo dmesg

dump:
	@echo "Dumping memory objects..."
	arm-linux-gnueabihf-objdump -h main.ko

e:
	@echo "Removing the module..."
	sudo rmmod main
	sudo dmesg


help:
	@echo "Usage:"
	@echo "  make modules         - Build the module"
	@echo "  make modules_install - Install the module"
	@echo "  make clean          - Clean the build files"
	@echo "  make help           - Show this help message"
	#make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) help






