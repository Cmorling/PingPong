obj-m += pingpong.o
pingpong-objs := main.o response.o handler.o file.o crypto.o

CFLAGS_pingpong.o := -D_FORTIFY_SOURCE=0

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean