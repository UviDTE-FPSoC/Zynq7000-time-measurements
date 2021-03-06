/*
 * main_functions.c
 *
 *  Created on: 28/07/2016
 *      Author: lucia
 */

#include <stdio.h>
#include <string.h> //to use memcpy
#include <inttypes.h>


#include "arm_cache_modified.h" //to use Legup cache config functions
#include "xil_cache.h"
#include "xil_mmu.h"
#include "xil_cache_l.h"

//--------------------------CACHE CONFIGURATION--------------------------//
void cache_configuration(int cache_config)
{
	if (cache_config<=0){ //0 no cache
		printf("\n\rCACHE CONFIG:0 No cache\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
	}else if (cache_config<=1){ //1 enable MMU
		printf("\n\rCACHE CONFIG:1 Enable MMU2\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		Xil_EnableMMU();
		printf("\n\rCACHE CONFIG:1 Enable MMU3\n\r");
	}else if (cache_config<=2){//2 do 1 and initialize L2C
		printf("\n\rCACHE CONFIG:2 Enable MMU and init L2C\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		Xil_EnableMMU();
		initialize_L2C();
	}else if (cache_config<=3){ //3 do 2 and enable SCU
		printf("\n\rCACHE CONFIG:3 Enable MMU and SCU and init L2C\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		Xil_EnableMMU();
		initialize_L2C();
		enable_SCU();
	}else if (cache_config<=4){ //4 do 3 and enable L1_I
		printf("\n\rCACHE CONFIG:4 L1_I\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_SCU();
//		enable_L1_I();
		Xil_L1ICacheEnable();
	}else if (cache_config<=5){ //5 do 4 and enable branch prediction
		printf("\n\rCACHE CONFIG:5 L1_I and branch prediction\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_SCU();
		enable_L1_I();
//		Xil_L1ICacheEnable();
		enable_branch_prediction();
	}else if (cache_config<=6){ // 6 do 5 and enable L1_D
		printf("\n\rCACHE CONFIG:6 L1_D, L1_I and branch prediction\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_SCU();
		enable_L1_D();
		enable_L1_I();
		Xil_L1DCacheEnable();
		Xil_L1ICacheEnable();
		enable_branch_prediction();
	}else if (cache_config<=7){ // 7 do 6 and enable L1 D side prefetch
		printf("\n\rCACHE CONFIG:7 L1_D with side prefetch), L1_I with branch prediction\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_SCU();
		enable_L1_D();
		enable_L1_I();
		enable_branch_prediction();
	}else if (cache_config<=8){ // 8 do 7 and enable L2C
		printf("\n\rCACHE CONFIG:8 L1_D side prefetch, L1_I with branch pre. and enable L2\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU(); // archivo bis
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_SCU();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}else if (cache_config<=9){ // 9 do 8 and enable L2 prefetch hint
		printf("\n\rCACHE CONFIG:9a basic config. + L2 prefetch hint\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_L2_prefetch_hint();
		enable_SCU();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}else if (cache_config<=10){ // 10 do 9 and enable write full line zeros
		printf("\n\rCACHE CONFIG:10 basic config. + L2ph + wr full line 0s\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_L2_prefetch_hint();
		enable_SCU();
		enable_write_full_line_zeros();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}else if (cache_config<=11){ // 11 do 10 and enable speculative linefills of L2 cache
		printf("\n\rCACHE CONFIG:11 basic config. + L2ph + wrfl0s + speculative linefills\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_L2_prefetch_hint();
		enable_SCU();
		enable_L2_speculative_linefill(); //call SCU first
		enable_write_full_line_zeros();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}else if (cache_config<=12){ // 12 do 11 and enable early BRESP
		printf("\n\rCACHE CONFIG:12 basic config. + L2ph + wrfl0s + sl + eBRESP\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_L2_prefetch_hint();
		enable_SCU();
		enable_L2_speculative_linefill(); //call SCU first
		enable_write_full_line_zeros();
		enable_early_BRESP();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}else if (cache_config<=13){ // 13 do 12 and store_buffer_limitation
		printf("\n\rCACHE CONFIG:13 basic config. + L2ph + wrfl0s + sl + eBRESP + buffer store limitation\n\r");
		Xil_DCacheDisable();
		Xil_ICacheDisable();
		Xil_DisableMMU();
		enable_MMU();
		initialize_L2C();
		enable_L1_D_side_prefetch();
		enable_L2_prefetch_hint();
		enable_SCU();
		enable_L2_speculative_linefill(); //call SCU first
		enable_write_full_line_zeros();
		enable_early_BRESP();
		enable_store_buffer_device_limitation();
		enable_caches(); //equivalent to enable L1_D,L1_I,branch prediction and L2
		Xil_L2CacheEnable_mod();
	}
}

//---------------EXTRA FUNCTIONS TO CALCULATE SOME STATISTICS IN EXPERIMENTS-------//
void reset_cumulative(unsigned long long int * total, unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance)
{
	*total = 0;
	*min = ~0;
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

	return (unsigned long long) vari;

	//return ((variance/(rep_tests-1))-(total/rep_tests)*(total/(rep_tests-1)));
}

