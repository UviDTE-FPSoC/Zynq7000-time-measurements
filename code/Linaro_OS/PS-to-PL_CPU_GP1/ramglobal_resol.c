#include <time.h>
//#include <user/librt.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

//#include "hwlib.h"
//#include "socal/socal.h"
//#include "socal/hps.h"
//#include "socal/alt_gpio.h"
#include <stdint.h>

#include <time.h>
#include "pmu.h"

void reset_cumulative(unsigned long long int * total, unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance);
void update_cumulative(unsigned long long int * total,  unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance, unsigned long long int ns_begin, unsigned long long int ns_end, unsigned long long int clk_read_delay);
unsigned long long variance (unsigned long long variance , unsigned long long total, unsigned long long rep_tests);

int main() {
  //import some constants from hps_0.h
	
	#define ON_CHIP_MEMORY_BASE 0xA7000000
	#define ON_CHIP_MEMORY_SPAN 4096 //size in bytes of memory
	//define variable type depending on on-chip RAM data width
	#define UINT_SOC uint32_t
	//constants to write and read memory
	#define BYTES_DATA_BUS 4
	#define MEMORY_SIZE_IN_WORDS (ON_CHIP_MEMORY_SPAN/BYTES_DATA_BUS) //number of 64bit words
	//macros to define hardware directions
 	#define MMAP_BASE 0x80000000 //default start of GP1
	#define MMAP_SPAN ON_CHIP_MEMORY_SPAN //Map just the memory size
	#define MMAP_MASK ( MMAP_SPAN - 1 )

        //#define REP_TESTS 1000
        #define CLK_REP_TESTS 1000

	//-----------COMMON VARIABLES------------//
	void *virtual_base;
	int fd;
	int REP_TESTS;

	void *on_chip_RAM_addr_void;

	int i, j, k, l, error;
	UINT_SOC* ocr_ptr;//on chip RAM pointer


    //---------Save intermediate results in speed tests------------//
    unsigned long long int total_clk, min_clk, max_clk, variance_clk, clk_read_average;

    unsigned long long int total_mcp_rd, min_mcp_rd, max_mcp_rd, variance_mcp_rd;
    unsigned long long int total_mcp_wr, min_mcp_wr, max_mcp_wr, variance_mcp_wr;

    unsigned long long int total_dir_rd, min_dir_rd, max_dir_rd, variance_dir_rd;
    unsigned long long int total_dir_wr, min_dir_wr, max_dir_wr, variance_dir_wr;

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

    UINT_SOC* datai; //pointer to data
	int data_in_one_operation_w;//data to write or read from memory (words)
	int operation_w_loops; //number of loops to read/write all data from/to memory


	//---------GENERATE ADRESSES----------//
	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span

	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, ON_CHIP_MEMORY_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, ON_CHIP_MEMORY_BASE);

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	on_chip_RAM_addr_void = virtual_base;
	UINT_SOC* on_chip_RAM_addr = (UINT_SOC *) on_chip_RAM_addr_void;



	//---------------------------------------------TESTS USING TIME.H---------------------------------------//
	printf("\n------------TESTS USING TIME.H------------\n");

	//-----------CLOCK INIT------------//
	clockid_t clk = CLOCK_REALTIME;
	struct timespec clk_struct_begin, clk_struct_end;

	 if (clock_getres(clk, &clk_struct_begin))
		 printf("Failed in checking CLOCK_REALTIME resolution");
	 else
		 printf("Clock CLOCK_REALTIME resolution is %ld ns\n", clk_struct_begin.tv_nsec );

	printf("Measuring clock read overhead\n");
	reset_cumulative(&total_clk, &min_clk, &max_clk, &variance_clk);
	clock_gettime(clk, &clk_struct_begin);
		for(i = 0; i<CLK_REP_TESTS; i++)
	{
		clock_gettime(clk, &clk_struct_begin);
		clock_gettime(clk, &clk_struct_end);

		update_cumulative(&total_clk, &min_clk, &max_clk, &variance_clk, clk_struct_begin.tv_nsec, clk_struct_end.tv_nsec, 0);
	}

	printf("Clock Statistics for %d consecutive reads\n", CLK_REP_TESTS);
	printf("Average, Minimum, Maximum, Variance\n");
	printf("%lld,%lld,%lld,%lld\n", clk_read_average = total_clk/CLK_REP_TESTS, min_clk, max_clk, variance (variance_clk , total_clk, CLK_REP_TESTS));


	//---------READ AND WRITE ON-CHIP RAM-------//

	//Check the memory
	ocr_ptr = on_chip_RAM_addr;
	error = 0;
	for (i=0; i<MEMORY_SIZE_IN_WORDS; i++)
	{
		*ocr_ptr = i;
		if (*ocr_ptr != i) error = 1;
		ocr_ptr++;
	}


	if (error == 0) printf("Check On-Chip RAM OK\n");
	else
	{
		printf ("Error when checking On-Chip RAM\n");
		return 1;
	}

	//Reset all memory
	ocr_ptr = on_chip_RAM_addr;
	error = 0;
	for (i=0; i<MEMORY_SIZE_IN_WORDS; i++)
	{
		*ocr_ptr = 0;
		if (*ocr_ptr != 0) error = 1;
		ocr_ptr++;
	}
	if (error == 0) printf("Reset On-Chip RAM OK\n");
	else
	{
		printf ("Error when resetting On-Chip RAM \n");
		return 0;
	}



	//----------Do the speed test (using memcpy)--------------//
	printf("\nStart memcpy test\n");

	printf("Data Size,Average MCP_WR,Average MCP_RD\n");

	for(i=0; i<number_of_data_sizes; i++)//for each data size
	{
		if (i<15) REP_TESTS=1000;
		else REP_TESTS=REP_TESTS/2;

		reset_cumulative( &total_mcp_wr, &min_mcp_wr, &max_mcp_wr, &variance_mcp_wr);
		reset_cumulative( &total_mcp_rd, &min_mcp_rd, &max_mcp_rd, &variance_mcp_rd);

		//reserve the exact memory of the data size
		data = (char*) malloc(data_size[i]);
		if (data == 0)
		{
			printf("ERROR when calling malloc: Out of memory\n");
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

		clock_gettime(clk, &clk_struct_begin);//read a value from clock to eliminate "first-read-jitter"
		clock_gettime(clk, &clk_struct_begin);
		for(l = 0; l<REP_TESTS; l++)
		{
			
			//WRITE DATA TO ON-CHIP RAM
			for (j=0; j<operation_loops; j++)
			memcpy((void*) on_chip_RAM_addr, (void *) (&(data[j*ON_CHIP_MEMORY_SPAN])), data_in_one_operation);
		}
		clock_gettime(clk, &clk_struct_end);
		unsigned long long int timeWR = ( clk_struct_end.tv_nsec < clk_struct_begin.tv_nsec ) ? 1000000000 - (clk_struct_begin.tv_nsec - clk_struct_end.tv_nsec) :														clk_struct_end.tv_nsec - clk_struct_begin.tv_nsec ;
		
		clock_gettime(clk, &clk_struct_begin);//read a value from clock to eliminate "first-read-jitter"
		clock_gettime(clk, &clk_struct_begin);
		for(l = 0; l<REP_TESTS; l++)
		{
			//READ DATA FROM ON-CHIP RAM
			for (j=0; j<operation_loops; j++)
			memcpy((void *) (&(data[j*ON_CHIP_MEMORY_SPAN])), (void*) on_chip_RAM_addr, data_in_one_operation);
		}
		clock_gettime(clk, &clk_struct_end);

		unsigned long long int timeRD = ( clk_struct_end.tv_nsec < clk_struct_begin.tv_nsec ) ? 1000000000 - (clk_struct_begin.tv_nsec - clk_struct_end.tv_nsec) :														clk_struct_end.tv_nsec - clk_struct_begin.tv_nsec ;
	
		//check the content of the data just read
		error = 0;
		for (j=0; j<data_size[i]; j++)
		{
			if (data[j] != i) error = 1;
		}
		if (error != 0)
		{
			printf ("Error when checking On-Chip RAM with data size %dB\n", data_size[i]);
			return 0;
		}
//		else
//			printf("Check On-Chip RAM with data size %dB was OK\n", data_size[i]);


		//free dynamic memory
		free(data);
		printf("%d, %lld, %lld\n", data_size[i], timeWR/REP_TESTS, timeRD/REP_TESTS);
	}

	//print or save in file timing measurements


	//----------Do the speed test (using for loop)--------------//
	printf("\nStart for loop test\n");
	printf("Data Size,Average MCP_WR,Average MCP_RD\n");

	for(i=0; i<number_of_data_sizes; i++)//for each data size
	{

		if (i<14) REP_TESTS=1000;
		else REP_TESTS=REP_TESTS/2;

		reset_cumulative( &total_dir_wr, &min_dir_wr, &max_dir_wr, &variance_dir_wr);
		reset_cumulative( &total_dir_rd, &min_dir_rd, &max_dir_rd, &variance_dir_rd);
		
		//reserve the exact memory of the data size (except when data is smaller than data bus size)
		if (data_size[i]<BYTES_DATA_BUS) datai = (UINT_SOC*) malloc(BYTES_DATA_BUS);
		else datai = (UINT_SOC*) malloc(data_size[i]);
		if (datai == 0)
		{
			printf("ERROR when calling malloc: Out of memory\n");
			return 1;
		}

		//save some content in data (for example: i)
		if (data_size[i]<BYTES_DATA_BUS) datai[0] = i;
		else for (j=0; j<(data_size[i]/BYTES_DATA_BUS); j++) datai[j] = i;

		//if data is bigger than on-chip size in bytes
		if (data_size[i]>ON_CHIP_MEMORY_SPAN)
		{
			data_in_one_operation_w = MEMORY_SIZE_IN_WORDS;
			operation_w_loops = (data_size[i]/BYTES_DATA_BUS)/MEMORY_SIZE_IN_WORDS;
		}
		else
		{
			if (data_size[i]<BYTES_DATA_BUS) //if data is smaller than bus width
			{
				data_in_one_operation_w = 1;
				operation_w_loops = 1;
			}
			else //data size is between data bus size and memory size
			{
				data_in_one_operation_w = data_size[i]/BYTES_DATA_BUS;
				operation_w_loops = 1;
			}
		}

		
		//read a value from clock to eliminate "first-read-jitter"
		clock_gettime(clk, &clk_struct_begin);
		clock_gettime(clk, &clk_struct_begin);
		
		int offset;
		for(l = 0; l<REP_TESTS; l++)
		{
			
			//WRITE DATA TO ON-CHIP RAM
			
			for (j=0; j<operation_w_loops; j++)
			{
				offset = j*MEMORY_SIZE_IN_WORDS;
				for(k=0; k<data_in_one_operation_w; k++)
					on_chip_RAM_addr[k] = datai[offset+k];
			}
		}
		clock_gettime(clk, &clk_struct_end);
		unsigned long long int timeWR = ( clk_struct_end.tv_nsec < clk_struct_begin.tv_nsec ) ? 1000000000 - (clk_struct_begin.tv_nsec - clk_struct_end.tv_nsec) :														clk_struct_end.tv_nsec - clk_struct_begin.tv_nsec ;
		
		clock_gettime(clk, &clk_struct_begin);
		clock_gettime(clk, &clk_struct_begin);
		for(l = 0; l<REP_TESTS; l++)
		{
			//READ DATA FROM ON-CHIP RAM
			for (j=0; j<operation_w_loops; j++)
			{
				offset = j*MEMORY_SIZE_IN_WORDS;
				for(k=0; k<data_in_one_operation_w; k++)
					datai[offset+k] = on_chip_RAM_addr[k];
			}
		}
		clock_gettime(clk, &clk_struct_end);
		unsigned long long int timeRD = ( clk_struct_end.tv_nsec < clk_struct_begin.tv_nsec ) ? 1000000000 - (clk_struct_begin.tv_nsec - clk_struct_end.tv_nsec) :														clk_struct_end.tv_nsec - clk_struct_begin.tv_nsec ;

		//check the content of the data just read
		error = 0;
		for (j=0; j<data_size[i]/BYTES_DATA_BUS; j++)
		{
			if (datai[j] != i) error = 1;
		}
		if (error != 0)
		{
			printf ("Error when checking On-Chip RAM with data size %dB\n", data_size[i]);
			return 0;

		}
		//else printf("Check On-Chip RAM with data size %dB was OK\n", data_size[i]);

		//free memory used
		free(datai);
		printf("%d, %lld, %lld\n", data_size[i], timeWR/REP_TESTS, timeRD/REP_TESTS);	
	}
	//--------------------------------------------- END OF TESTS USING TIME.H---------------------------------------//



	// ------clean up our memory mapping and exit -------//
	if( munmap( virtual_base, MMAP_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}

// -----------------------------------EXTRA FUNCTIONS----------------------------------------------------//
void reset_cumulative(unsigned long long int * total, unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance)
{
	*total = 0;
	*min = (signed int)(1000000000);
	*max = 0;
	*variance = 0;
}

void update_cumulative(unsigned long long int * total,  unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance, unsigned long long int ns_begin, unsigned long long int ns_end, unsigned long long int clk_read_delay)
{
	unsigned long long int tmp = ( ns_end < ns_begin ) ? 1000000000 - (ns_begin - ns_end) - clk_read_delay :
														ns_end - ns_begin - clk_read_delay ;

	*total = *total + tmp;
	*variance = *variance + tmp*tmp;

	if (tmp < *min) *min = tmp;
	if (tmp > *max) *max = tmp;

//	printf("total %lld, begin %lld, end %lld\n", *total, ns_begin, ns_end);
}

unsigned long long variance (unsigned long long variance , unsigned long long total, unsigned long long rep_tests)
{

	float media_cuadrados, quef1, quef2, cuadrado_media, vari;

	media_cuadrados = (variance/(float)(rep_tests-1));
	quef1 = (total/(float)rep_tests);
	quef2=(total/(float)(rep_tests-1));
	cuadrado_media = quef1 * quef2;
	vari = media_cuadrados - cuadrado_media;
/*
	printf("media_cuadrados %f,",media_cuadrados );
	printf("quef1 %f,",quef1 );
	printf("quef2 %f,",quef2 );
	printf("cuadrado_media %f,",cuadrado_media );
	printf("variance %f\n",vari );
*/
	return (unsigned long long) vari;

	//return ((variance/(rep_tests-1))-(total/rep_tests)*(total/(rep_tests-1)));
}
