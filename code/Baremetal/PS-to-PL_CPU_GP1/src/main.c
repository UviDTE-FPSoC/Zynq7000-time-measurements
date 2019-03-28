/*
 * main.c
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



/* enable semihosting with gcc by defining an __auto_semihosting symbol */
int __auto_semihosting;


#define REP_TESTS 100 //repetitions of every time measurement
#define CLK_REP_TESTS 1000

#define FORLOOP //Uncomment if you want to measure transfers using for loop (direct method)

//----------------------CACHE CONFIGURATION------------------------//
#define CACHE_CONFIG 0
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
	#define ON_CHIP_MEMORY_SPAN 0x4000 //size in bytes of memory
	#define ONCHIP_MEMORY_END ONCHIP_MEMORY2_0_END
	//define variable type depending on on-chip RAM data width
	#define UINT_SOC uint32_t
	//constants to write and read memory
	#define BYTES_DATA_BUS 4
	#define MEMORY_SIZE_IN_WORDS (ON_CHIP_MEMORY_SPAN/BYTES_DATA_BUS) //number of 32bit words
	//macros to define hardware directions
	#define BRIDGE_BASE 0x80000000 //default start for HPS-FPGA High


	printf("-----MEASURING FPGA-HPS BRIDGES SPEED IN BAREMETAL----\n\r\r");

	printf("Each measurement is repeated %d times\n\r", REP_TESTS);

	//------------------------VARIABLES INSTANTIATION-----------------------//
	//-------------Common variables------------//
	ALT_STATUS_CODE status = ALT_E_SUCCESS;

	int i, j, l; //vars for loops


	//cache configuration
	int cache_config = CACHE_CONFIG;


	//-------------Memory pointers------------//
	//FPGA On-Chip RAM
	void* FPGA_on_chip_RAM_addr = (void*)(BRIDGE_BASE + ON_CHIP_MEMORY_BASE); //address of FPGA OCR
	UINT_SOC* fpgaocr_ptr;//FPGA on chip RAM pointer

	//----Save intermediate results in speed tests-------//
	unsigned long long int total_clk, min_clk, max_clk, variance_clk, clk_read_average;

	unsigned long long int total_mcp_rd, min_mcp_rd, max_mcp_rd, variance_mcp_rd;
	unsigned long long int total_mcp_wr, min_mcp_wr, max_mcp_wr, variance_mcp_wr;

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
	int k;
	unsigned long long int total_dir_rd, min_dir_rd, max_dir_rd, variance_dir_rd;
	unsigned long long int total_dir_wr, min_dir_wr, max_dir_wr, variance_dir_wr;
	UINT_SOC* datai; //pointer to data
	int data_in_one_operation_w;//data to write or read from memory (words)
	int operation_w_loops; //number of loops to read/write all data from/to memory
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
		if (i>=2) update_cumulative(&total_clk, &min_clk, &max_clk, &variance_clk, 0, pmu_counter_ns, 0);
		//(We erase two first measurements because they are different from the others. Reason:Branch prediction misses when entering for loop)
	}
	printf("PMU Cycle Timer Statistics for %d consecutive reads\n\r", CLK_REP_TESTS);
	printf("Average, Minimum, Maximum, Variance\n\r");
	printf("%lld,%lld,%lld,%lld\n\r", clk_read_average = total_clk/CLK_REP_TESTS, min_clk, max_clk, variance (variance_clk , total_clk, CLK_REP_TESTS));



	//-----------MOVING DATA WITH PROCESSOR------------//
	printf("\n\r--MOVING DATA WITH THE PROCESSOR--\n\r");
	//----------Do the speed test (using memcpy)--------------//
	printf("\n\rStart memcpy test\n\r");

	printf("Data Size,Average MCP_WR,Minimum MCP_WR,Maximum MCP_WR,Variance MCP_WR,Average MCP_RD,Minimum MCP_RD,Maximum MCP_RD,Variance MCP_RD\n\r");

	for(i=0; i<number_of_data_sizes; i++)//for each data size
	{
		reset_cumulative( &total_mcp_wr, &min_mcp_wr, &max_mcp_wr, &variance_mcp_wr);
		reset_cumulative( &total_mcp_rd, &min_mcp_rd, &max_mcp_rd, &variance_mcp_rd);

		for(l = 0; l<REP_TESTS+2; l++)
		{
			//reserve the exact memory of the data size
			data = (char*) malloc(data_size[i]);
			if (data == 0)
			{
				printf("ERROR when calling malloc: Out of memory2\n\r");
				return 1;
			}

			//save some content in data (for example: i)
			for (j=0; j<data_size[i]; j++) data[j] = i;

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

			//WRITE DATA TO ON-CHIP RAM

			pmu_counter_reset();

			for (j=0; j<operation_loops; j++)
				memcpy((void*) FPGA_on_chip_RAM_addr, (void *) (&(data[j*ON_CHIP_MEMORY_SPAN])), data_in_one_operation);

			overflow = pmu_counter_read_ns(&pmu_counter_ns);
			if (overflow == 1){printf("Cycle counter overflow!! Program ended\n\r");return 1;}
			//printf("PMU counter (ns): %lld \n\r", pmu_counter_ns);

			if (l>=2) update_cumulative(&total_mcp_wr, &min_mcp_wr, &max_mcp_wr, &variance_mcp_wr, 0, pmu_counter_ns, clk_read_average);
			//(We erase two first measurements because they are different from the others. Reason:Branch prediction misses when entering for loop)


			//READ DATA FROM ON-CHIP RAM

			pmu_counter_reset();

			for (j=0; j<operation_loops; j++)
				memcpy((void *) (&(data[j*ON_CHIP_MEMORY_SPAN])), (void*) FPGA_on_chip_RAM_addr, data_in_one_operation);


			overflow = pmu_counter_read_ns(&pmu_counter_ns);
			if (overflow == 1){printf("Cycle counter overflow!! Program ended\n\r");return 1;}

			if (l>=2) update_cumulative(&total_mcp_rd, &min_mcp_rd, &max_mcp_rd, &variance_mcp_rd, 0, pmu_counter_ns, clk_read_average);
			//(We erase two first measurements because they are different from the others. Reason:Branch prediction misses when entering for loop)

			//check the content of the data just read
			for (j=0; j<data_size[i]; j++)
			{
				if (data[j] != i) status = ALT_E_ERROR;
			}
			if (status != ALT_E_SUCCESS)
			{
				printf ("Error when checking On-Chip RAM with data size %dB\n\r", data_size[i]);
				return 1;
			}

			//free dynamic memory
			free(data);

		}

		printf("%d, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld\n\r", data_size[i], total_mcp_wr/REP_TESTS, min_mcp_wr, max_mcp_wr, variance(variance_mcp_wr, total_mcp_wr, REP_TESTS), total_mcp_rd/REP_TESTS, min_mcp_rd, max_mcp_rd,  variance(variance_mcp_rd, total_mcp_rd, REP_TESTS) );
	}

	//print or save in file timing measurements

	printf("\n\rData transfer measurements finished!!!!\n\r");

    return 0;
}
