obj-m := elevator.o

PWD := $(shell pwd)
KDIR := /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.o *.ko *.mod.* Module.* modules.*

test:
	sudo dmesg -C
	sudo insmod elevator.ko
	sudo cat /proc/elevator
	sudo rmmod elevator.ko
	sudo dmesg > debug.txt
	