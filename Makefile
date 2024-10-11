obj-m += key_press_counter.o

all:
	make -C ./vroot/lib/modules/6.7.4/build M=$(PWD) modules

clean:
	make -C ./vroot/lib/modules/6.7.4/build M=$(PWD) clean
