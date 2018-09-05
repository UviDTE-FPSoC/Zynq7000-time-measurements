CC = arm-xilinx-linux-gnueabi-gcc
CFLAGS = -g

all : ramglobal_resol 

led_blink : ramglobal_resol.o
	${CC} ${CFLAGS} -o $^ $@

clean :
	rm -rfv *.o
	rm -rfv ramglobal_resol 

.PHONY : clean
