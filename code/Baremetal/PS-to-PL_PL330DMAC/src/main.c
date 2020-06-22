/* main.c

 *

 *  Created on: 28/07/2016

 *      Author: lucia

 */





#include <stdio.h> //printf

#include <string.h> //to use memcpy

#include <stdlib.h> // to use malloc and free

#include <inttypes.h>

#include "hwlib.h"

#include "main_functions.h"

#include "pmu.h"

#include "xdmaps_hw.h"

#include "xdmaps_mod.h"

#include "xscugic.h"

#include "xilinxdma_sa.h"





/* enable semihosting with gcc by defining an __auto_semihosting symbol */

int __auto_semihosting;





#define REP_TESTS 100 //repetitions of every time measurement

#define CLK_REP_TESTS 1000



#define FORLOOP //Uncomment if you want to measure transfers using for loop (direct method)



//----------------------CACHE CONFIGURATION------------------------//

#define CACHE_CONFIG 9

/*Options for cache config (each config is added to the all previous ones):

0 no cache

(basic config and optimizations)

1 enable MMU

2 do 1 and initialize L2C

3 do 2 and enable SCU

4 do 3 and enable L1_I

5 do 4 and enable branch prediction

6 do 5 and enable L1_D

7 do 6 and enable L1 D side prefetch

8 do 7 and enable L2C

(special L2C-310 controller + Cortex A9 optimizations)

9 do 8 and enable L2 prefetch hint

10 do 9 and enable write full line zeros

11 do 10 and enable speculative linefills of L2 cache

12 do 11 and enable early BRESP

13 do 12 and store_buffer_limitation

*/



int main(void)

{



	#define ON_CHIP_MEMORY_BASE 0x00000000

	#define ON_CHIP_MEMORY_SPAN 0x20000 //size in bytes of memory

	#define ONCHIP_MEMORY_END ONCHIP_MEMORY2_0_END

	//define variable type depending on on-chip RAM data width

	#define UINT_SOC uint32_t

	//constants to write and read memory

	#define BYTES_DATA_BUS 4

	#define MEMORY_SIZE_IN_WORDS (ON_CHIP_MEMORY_SPAN/BYTES_DATA_BUS) //number of 32bit words

	//macros to define hardware directions

	#define BRIDGE_BASE 0x80000000 //default start for HPS-FPGA High



	#define NUM_DMA_PROGS	(2*1024*1024/ON_CHIP_MEMORY_SPAN)



	printf("-----MEASURING FPGA-HPS BRIDGES SPEED IN BAREMETAL----\n\r\r");



	printf("Each measurement is repeated %d times\n\r", REP_TESTS);



	//------------------------VARIABLES INSTANTIATION-----------------------//

	//-------------Common variables------------//

	ALT_STATUS_CODE status = ALT_E_SUCCESS;



	int i, j, l; //vars for loops



	//cache configuration

	int cache_config = CACHE_CONFIG;





	//DMA ones

	XDmaPs DmaInstance;

	char* wr_dst;

	char* wr_src;

	char* rd_dst;

	char* rd_src;

	int Status;

	XDmaPs_Cmd *DmaCmd_wr[NUM_DMA_PROGS];

	for (j=0; j<NUM_DMA_PROGS;j++){

		DmaCmd_wr[j]=(XDmaPs_Cmd*) malloc(sizeof(XDmaPs_Cmd));

	}



	wr_dst = (char*) 0x80000000;

	rd_src = (char*) 0x80000000;



	XDmaPs *DmaInstanceA[NUM_DMA_PROGS];

	XScuGic GicInstance;

	u32 DmaProg;

	u32 DmaProgA[NUM_DMA_PROGS];

	int Checked[XDMAPS_CHANNELS_PER_DEV];

	unsigned int Channel;



	for (j=0; j<NUM_DMA_PROGS;j++){

		DmaInstanceA[j]=(XDmaPs*) malloc(sizeof(XDmaPs));

	}



	//-------------Memory pointers------------//

	//FPGA On-Chip RAM

	void* FPGA_on_chip_RAM_addr = (void*)(BRIDGE_BASE + ON_CHIP_MEMORY_BASE); //address of FPGA OCR

	UINT_SOC* fpgaocr_ptr;//FPGA on chip RAM pointer



	//----Save intermediate results in speed tests-------//

	unsigned long long int total_clk, min_clk, max_clk, variance_clk, clk_read_average;

	unsigned long long int total_dma_rd, min_dma_rd, max_dma_rd, variance_dma_rd, temp_rd, temp;

	unsigned long long int total_dma_wr, min_dma_wr, max_dma_wr, variance_dma_wr, temp_wr;



	//-----------some variables to define speed tests-------//

	//define data sizes

	int number_of_data_sizes = 21; //size of data_size[]

	int data_size [] =

	{2, 4, 8, 16, 32, 64, 128, 256, 512, 1024,

	2048, 4096, 8192, 16384, 32768,

	65536, 131072, 262144, 524288, 1048576,

	2097152}; //data size in bytes



	//-----------auxiliar variables to perform speed tests-------//

	char* data;

	int data_in_one_operation;//data to write or read from memory (bytes)

	int operation_loops; //number of loops to read/write all data from/to memory





	//Variables if For Loop Defined

	#ifdef FORLOOP

	unsigned long long int total_dir_rd, min_dir_rd, max_dir_rd, variance_dir_rd;

	unsigned long long int total_dir_wr, min_dir_wr, max_dir_wr, variance_dir_wr;

	#endif



	//-------------CHECKING FPGA 256kB ON-CHIP RAM-------------//



	//Check the memory

	fpgaocr_ptr = (UINT_SOC *)FPGA_on_chip_RAM_addr;

	for (i=4; i<(MEMORY_SIZE_IN_WORDS); i++)

	{

		*fpgaocr_ptr = i;

		if (*fpgaocr_ptr != i){

			status = ALT_E_ERROR;

			printf("i during error %d\n\r", i);

		}

		fpgaocr_ptr++;

	}

	if (status == ALT_E_SUCCESS) printf("Check FPGA On-Chip RAM OK\n\r");

	else

	{

		printf ("Error when checking FPGA On-Chip RAM\n\r");

		return 1;

	}

	//Reset all memory


	fpgaocr_ptr = (UINT_SOC *)FPGA_on_chip_RAM_addr;

	for (i=0; i<MEMORY_SIZE_IN_WORDS; i++)

	{

		*fpgaocr_ptr = 0;

		if (*fpgaocr_ptr != 0) status = ALT_E_ERROR;;

		fpgaocr_ptr++;

	}

	if (status == ALT_E_SUCCESS) printf("Reset FPGA On-Chip RAM OK\n\r");

	else

	{

		printf ("Error when resetting FPGA On-Chip RAM \n\r");

		return 1;

	}





	//----------CONFIGURING CACHE-----------//

	cache_configuration(cache_config);





	//---------------------------------------------TESTS USING PMU---------------------------------------//

	printf("\n\r------------STARTING TIME MEASUREMENTS------------\n\r");



	unsigned long long pmu_counter_ns;

	int overflow = 0;



	//-----------INITIALIZATION OF PMU AS TIMER------------//

	pmu_init_ns(667, 1); //Initialize PMU cycle counter, 667MHz source, frequency divider 1

	pmu_counter_enable();//Enable cycle counter inside PMU (it starts counting)

	float pmu_res = pmu_getres_ns();

	printf("PMU is used like timer with the following characteristics\n\r");

	printf("PMU cycle counter resolution is f ns\n\r" );

	printf("Measuring PMU counter overhead...\n\r");

	reset_cumulative(&total_clk, &min_clk, &max_clk, &variance_clk);

	for(i = 0; i<CLK_REP_TESTS+2; i++)

	{

		pmu_counter_reset();

		overflow = pmu_counter_read_ns(&pmu_counter_ns);

		if (overflow == 1){printf("Cycle counter overflow!! Program ended\n\r");return 1;}

		if (i>=2) update_cumulative(&total_clk, 0, pmu_counter_ns, 0, &temp);

		//(We erase two first measurements because they are different from the others. Reason:Branch prediction misses when entering for loop)

	}

	printf("PMU Cycle Timer Statistics for %d consecutive reads\n\r", CLK_REP_TESTS);

	printf("Average, Minimum, Maximum, Variance\n\r");

	printf("%lld,%lld,%lld,%lld\n\r", clk_read_average = total_clk/CLK_REP_TESTS, min_clk, max_clk, variance (variance_clk , total_clk, CLK_REP_TESTS));





	//-----------MOVING DATA WITH DMAC------------//

	printf("\n\r--MOVING DATA WITH THE DMAC--\n\r");





	printf("\n\r--Starting DMAC tests (DMAC program preparation time included)--\n\r");

	//Moving data with DMAC (DMAC program preparation is measured)

	printf("Data Size,Average DMA_WR,Minimum DMA_WR,Maximum DMA_WR,Variance DMA_WR,Average DMA_RD,Minimum DMA_RD,Maximum DMA_RD,Variance DMA_RD\n\r");

    for(i=0; i<number_of_data_sizes; i++)//for each data size

	{

		reset_cumulative( &total_dma_wr, &min_dma_wr, &max_dma_wr, &variance_dma_wr);

		reset_cumulative( &total_dma_rd, &min_dma_rd, &max_dma_rd, &variance_dma_rd);





		//if data is bigger than on-chip size in bytes

		if (data_size[i]>ON_CHIP_MEMORY_SPAN)

		{

			data_in_one_operation = ON_CHIP_MEMORY_SPAN;

			operation_loops = data_size[i]/ON_CHIP_MEMORY_SPAN;

		}

		else

		{

			data_in_one_operation = data_size[i];

			operation_loops = 1;

		}





		//reserve the exact memory of the data size

		data = (char*) malloc(data_size[i]);

		if (data == 0)

		{

			printf("ERROR when calling malloc: Out of memory\n\r");

			return 1;

		}



		wr_src = data;



		//save some content in data (for example: i)

		for (j=0; j<data_size[i]; j++) data[j] = i;



		for (j=0; j<operation_loops; j++)

		{

			//write programs

			XDmaSA_Program((void*)(&(wr_src[j*ON_CHIP_MEMORY_SPAN])), (void*)wr_dst, (size_t) data_in_one_operation, DmaCmd_wr[j]);

		}

		Channel=0;



		for(l = 0; l<REP_TESTS+2; l++)

		{



			temp_wr=0;

			for (j=0; j<operation_loops; j++)

			{

				Status =  XDmaSA_Init(&GicInstance, DmaInstanceA[j], DMA_DEVICE_ID);

				if (Status != XST_SUCCESS) {

					xil_printf("Error: XDMaPs_Example_Init failed\r\n");

					return XST_FAILURE;

				}



				Status = XDmaPs_Start_Program(DmaInstanceA[j], Channel, DmaCmd_wr[j], 0, &(DmaProgA[j]));

								if (Status != XST_SUCCESS) {

									printf("XDmaPs_Start_Transfer_Program error\n\r");

									return XST_FAILURE;

								}



				XDmaPs_SetDoneHandler(DmaInstanceA[j], Channel, DmaDoneHandler, (void *)Checked);



				Checked[Channel] = 0;



				DmaProg=DmaProgA[j];

				DmaInstance=*DmaInstanceA[j];



				if (DmaProg){

					pmu_counter_reset();

					Status = XDmaPs_Exec_DMAGO(DmaInstanceA[j]->Config.BaseAddress, Channel, DmaProgA[j]);

					if (Status != XST_SUCCESS)

						printf("XDmaPs_Exec_DMAGO error\n\r");

					while (Checked[Channel] == 0){ //wait DMAC to finish

					}

					overflow = pmu_counter_read_ns(&pmu_counter_ns);

					if (overflow == 1){printf("Cycle counter overflow!! Program ended\n\r");return 1;}



					if (l>=2)update_cumulative(&total_dma_wr, 0, pmu_counter_ns, clk_read_average, &temp_wr);

					if ((l>=2) && (j==(operation_loops-1)))  update_cumulative2(temp_wr, &min_dma_wr, &max_dma_wr, &variance_dma_wr);

				}

				else {

					DmaInstance.Chans[Channel].DmaCmdToHw = NULL;

					Status = XST_FAILURE;

					xil_printf("Error: Aqui2\r\n");

				}



			}



			//check the content of the data just read

			// Compare results



			if(0  != memcmp(&(data[0]), (void*) wr_dst, data_in_one_operation))

			{

				printf("DMA source and destiny have different data on WR!! Program ended\n\r");return 1;

			}



// Read operation (DMA)



	rd_dst = data;

	for (j=0; j<data_size[i]; j++) data[j] = i+1;

	for (j=0; j<operation_loops; j++)

	{

		//read programs

		XDmaSA_Program( (void*)rd_src, (void*)(&(rd_dst[j*ON_CHIP_MEMORY_SPAN])), (size_t) data_in_one_operation, DmaCmd_wr[j]);

	}



	temp_rd=0;

	for (j=0; j<operation_loops; j++)

	{





		Status =  XDmaSA_Init(&GicInstance, DmaInstanceA[j], DMA_DEVICE_ID);

		if (Status != XST_SUCCESS) {

			xil_printf("Error: XDMaPs_Example_Init failed\r\n");

			return XST_FAILURE;

		}





		Status = XDmaPs_Start_Program(DmaInstanceA[j], Channel, DmaCmd_wr[j], 0, &(DmaProgA[j]));

						if (Status != XST_SUCCESS) {

							printf("XDmaPs_Start_Transfer_Program error\n\r");

							return XST_FAILURE;

						}



		XDmaPs_SetDoneHandler(DmaInstanceA[j], Channel, DmaDoneHandler, (void *)Checked);



		Checked[Channel] = 0;



		DmaProg=DmaProgA[j];

		DmaInstance=*DmaInstanceA[j];



		if (DmaProg){

			pmu_counter_reset();

			Status = XDmaPs_Exec_DMAGO(DmaInstanceA[j]->Config.BaseAddress, Channel, DmaProgA[j]);

			if (Status != XST_SUCCESS)

				printf("XDmaPs_Exec_DMAGO error\n\r");

			while (Checked[Channel] == 0){ //wait DMAC to finish

			}

			overflow = pmu_counter_read_ns(&pmu_counter_ns);

			if (overflow == 1){printf("Cycle counter overflow!! Program ended\n\r");return 1;}



			if (l>=2) update_cumulative(&total_dma_rd, 0, pmu_counter_ns, clk_read_average, &temp_rd);

			if ((l>=2) && (j==(operation_loops-1)))  update_cumulative2(temp_rd, &min_dma_rd, &max_dma_rd, &variance_dma_rd);

		}

		else {

			DmaInstance.Chans[Channel].DmaCmdToHw = NULL;

			Status = XST_FAILURE;

			xil_printf("Error\r\n");

		}



	}



	//check the content of the data just read

	// Compare results

				if(0  != memcmp(&(data[0]), (void*) rd_src, data_in_one_operation))

				{

					printf("DMA source and destiny have different data on RD!! Program ended\n\r");return 1;

				}

		} // REP_TEST

//		printf("Check with data size %dB was OK aqui\n\r", data_size[i]);

		printf("%d, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld\n\r", data_size[i], total_dma_wr/REP_TESTS, min_dma_wr, max_dma_wr, variance(variance_dma_wr, total_dma_wr, REP_TESTS), total_dma_rd/REP_TESTS, min_dma_rd, max_dma_rd,  variance(variance_dma_rd, total_dma_rd, REP_TESTS) );



		//free dynamic memory

		free(data);

	}



	printf("\n\rData transfer measurements finished!!!!\n\r");



    return 0;

}
