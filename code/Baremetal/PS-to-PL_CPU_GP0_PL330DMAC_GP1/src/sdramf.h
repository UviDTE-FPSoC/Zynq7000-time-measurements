/*
 * sdramf.h
 *
 *  Created on: 10/11/2017
 *      Author: lucia
 */

#ifndef SDRAMF_H_
#define SDRAMF_H_


//function to enable page match, urgent, aging and configure aging counters in ports (8)
void set_sdramc_config_ports(unsigned int addrconfig, unsigned int  config);
//function to disable page match, urgent, aging and configure aging counters in ports (8)
void reset_sdramc_config_ports(unsigned int addrconfig, unsigned int  config);
// function to determine the source of urgent configuration and to activate urgent option in ports (8)
void set_sdramc_urgent(unsigned int  sourceconfig, unsigned int  source);
// function to disable page match
void disable_sdramc_page_match();


#endif /* SDRAMF_H_ */
