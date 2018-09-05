/*
 * main_functions.h
 *
 *  Created on: 28/07/2016
 *      Author: lucia
 */

#ifndef MAIN_FUNCTIONS_H_
#define MAIN_FUNCTIONS_H_


//----------------------CACHE CONFIGURATION-----------------------//
void cache_configuration(int cache_config);

//---------------------------ACP CONFIGURATION---------------------------//


//---------------EXTRA FUNCTIONS TO CALCULATE SOME STATISTICS IN EXPERIMENTS-------//
void reset_cumulative(unsigned long long int * total, unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance);
void update_cumulative(unsigned long long int * total,  unsigned long long int* min, unsigned long long int * max, unsigned long long int * variance, unsigned long long int ns_begin, unsigned long long int ns_end, unsigned long long int clk_read_delay);
unsigned long long variance (unsigned long long variance , unsigned long long total, unsigned long long rep_tests);




#endif /* MAIN_FUNCTIONS_H_ */
