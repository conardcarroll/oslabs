KVERSION := $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build
PWD := $(shell pwd)
obj-m := interface.o

all: interface.c app.c
	gcc -O3 app.c -o app -lpthread 
	$(MAKE) -C $(KDIR) M=$(PWD) modules
