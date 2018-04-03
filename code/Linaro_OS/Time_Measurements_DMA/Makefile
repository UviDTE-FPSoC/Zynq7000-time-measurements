CROSS_COMPILE := arm-xilinx-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
ARCH= arm
LDFLAGS =  -g -Wall
time_measurements_DMA: time_measurements_DMA.o pmu.o
	$(CC) -o time_measurements_DMA time_measurements_DMA.o pmu.o

time_measurements_DMA.o: time_measurements_DMA.c pmu.h
	$(CC) -c time_measurements_DMA.c

pmu.o: pmu.c pmu.h
	$(CC) -c pmu.c
