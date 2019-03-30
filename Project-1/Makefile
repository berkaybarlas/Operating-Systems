gcc:
	gcc -o shelldon.o shelldon.c
shell:
	gcc -o shelldon.o shelldon.c && ./shelldon.o
testo: 
	gcc -o test.o test.c && ./test.o

obj-m	:= oldestChild.o
 
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
 
all: default
 
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
 
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions