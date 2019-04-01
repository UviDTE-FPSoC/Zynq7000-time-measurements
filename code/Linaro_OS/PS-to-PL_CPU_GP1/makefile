CC = arm-xilinx-linux-gnueabi-gcc
CFLAGS = -g

all : time_measurements_CPU

led_blink : time_measurements_CPU.o
	${CC} ${CFLAGS} -o $^ $@

clean :
	rm -rfv *.o
	rm -rfv time_measurements_CPU 

.PHONY : clean
