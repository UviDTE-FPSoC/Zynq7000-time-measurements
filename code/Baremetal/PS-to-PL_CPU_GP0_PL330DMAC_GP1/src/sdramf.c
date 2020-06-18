/*
 * sdramf.c
 *
 *  Created on: 08/11/2017
 *      Author: lucia
 */



#include <stdio.h>

//SDRAM controller address
#define SDRREGS 0xF8006000
#define SDRREGS_SPAN 0x20000 //128kB
//Offset of the internal registers in SDRAM controller
#define LOCK_KEY 0xF8000004
#define UNLOCK_KEY 0xF8000008
#define MURGENT_SEL 0xF800061C
#define MURGENT_VAL 0xF8000600
#define PAGE_MASK 0x00000204
#define RD_PORT_BASE 0x218
#define WR_PORT_BASE 0x208
#define MPWEIGHT_0_4 0x50B0
#define MPWEIGHT_1_4 0x50B4

void set_sdramc_config_ports(unsigned int addrconfig, unsigned int  config)
{
	unsigned int tmp, tmp_config;

    //config 1XXX111111XXXXXXXXXX FOR WR PORTS XXXX111111XXXXXXXXXX FOR RD PORTS

	  tmp = *((unsigned int *)(SDRREGS + addrconfig));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp | config;
//	  *((unsigned int *)(SDRREGS + addrconfig)) = tmp_config;
	  return;
}
void reset_sdramc_config_ports(unsigned int addrconfig, unsigned int  config)
{
	unsigned int tmp, tmp_config, tmp_config1, tmp00;

    //config 1XXX111111XXXXXXXXXX FOR WR PORTS XXXX111111XXXXXXXXXX FOR RD PORTS
	  tmp = *((unsigned int *)(0xF8006208));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp | 0x10000;
	  *((unsigned int *)(0xF8006208)) = tmp_config;


	  tmp = *((unsigned int *)(0xF8006218));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp | 0x10000;
	  *((unsigned int *)(0xF8006218)) = tmp_config;


	  tmp = *((unsigned int *)(0xF800620C));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp | 0x10000;
	  *((unsigned int *)(0xF800620C)) = tmp_config;


	  tmp = *((unsigned int *)(0xF800621C));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp | 0x10000;
	  *((unsigned int *)(0xF800621C)) = tmp_config;



	  tmp = *((unsigned int *)(SDRREGS + addrconfig));
	  printf("Conf reg  %x \n\r", tmp);
	  tmp_config = tmp & config;
	  *((unsigned int *)(SDRREGS + addrconfig)) = tmp_config;
	  tmp_config1 =  *((unsigned int *)(SDRREGS + addrconfig));
	  printf("Conf reg after %x \n\r", tmp_config1);

	  tmp00 = *((unsigned int *)(0xF8006208));
	  printf("PORT0W after %x\n\r", tmp00);
	  tmp00 = *((unsigned int *)(0xF800620C));
	  printf("PORT1W after %x\n\r", tmp00);
	  tmp00 = *((unsigned int *)(0xF8006218));
	  printf("PORT0R after %x\n\r", tmp00);
	  tmp00 = *((unsigned int *)(0xF800621C));
	  printf("PORT1R after %x\n\r", tmp00);

	  return;
}
void set_sdramc_urgent(unsigned int  sourceconfig, unsigned int  urgentconfig)
{
	unsigned int tmp0, tmp_sel, tmp, tmp_val0, tmp_val, tmp00, tmp1;

	*((unsigned int *)(UNLOCK_KEY)) = 0xDF0D;

	//sourceconfig 0x11110000
  tmp = *((unsigned int *)(MURGENT_SEL));
  tmp_sel = tmp & sourceconfig;
//  tmp_sel = tmp & 0x00000011;

  printf("Urgent_sel  %x, %x\n\r", tmp, tmp_sel);

  *((unsigned int *)(MURGENT_SEL)) = tmp_sel;

  tmp1 = *((unsigned int *)(MURGENT_SEL));

  printf("Urgent_sel after %x\n\r", tmp1);

  tmp0 = *((unsigned int *)(MURGENT_VAL));
  tmp_val0 = tmp & 0x11111100;
  tmp_val = tmp_val0 | urgentconfig;

  printf("Urgent_val  %x, %x, %x\n\r", tmp0, tmp_val0, tmp_val);

 //  *((unsigned int *)(MURGENT_VAL)) = tmp_val;

  *((unsigned int *)(MURGENT_VAL))= 0x00000022;

  tmp00 = *((unsigned int *)(MURGENT_VAL));

  printf("Urgent_val after %x\n\r", tmp00);

	*((unsigned int *)(LOCK_KEY)) = 0x767B;

    //sourceconfig 0xFFFFFFXX

  tmp00 = *((unsigned int *)(0xF8006208));
  printf("PORT0W after %x\n\r", tmp00);
  tmp00 = *((unsigned int *)(0xF800620C));
  printf("PORT1W after %x\n\r", tmp00);
  tmp00 = *((unsigned int *)(0xF8006218));
  printf("PORT0R after %x\n\r", tmp00);
  tmp00 = *((unsigned int *)(0xF800621C));
  printf("PORT1R after %x\n\r", tmp00);


  return;
}

void disable_sdramc_page_match()
{
  *((unsigned int *)(SDRREGS + PAGE_MASK )) = 0;
  return;
}
