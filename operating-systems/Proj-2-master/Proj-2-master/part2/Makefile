ifneq ($(KERNELRELEASE),)
	obj-m := my_timer.o
else
	KERNELDIR ?= /lib/modules/`uname -r`/build/
	PWD := `pwd`
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -f *.ko *.o Module* *mod*

test:
	sudo insmod my_timer.ko
	sudo cat /proc/timed
	sleep 1
	sudo cat /proc/timed
	sleep 2
	sudo cat /proc/timed
	sleep 5
	sudo cat /proc/timed
	sudo rmmod my_timer.ko
