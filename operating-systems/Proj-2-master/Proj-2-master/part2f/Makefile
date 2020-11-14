#\\\\\\\\\\\                                                              >
#=========================================================================>>>>
#///////////                                                              >

#STUDENTS    : Zachary Berrhill, Chris Lopes, James Roquebert.
#CLASS       : COP 4610 Intro To Operating Systems.
#ASSIGNMENT  : Project 2, Part II
#DATE        : 03/04/2020

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


