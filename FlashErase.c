/*****************************************************************

		sBOX (DSK6713)  FLASH TOOL PROGRAM
		running on CCS

    	File Name   :   flasherase.c
    	Ver.        :   1.0.0
    	Date        :   2006. 5. 15

	FLASH ROMを消去

    Copyright(c) 2006. Mtt Company DSP Lab. Kobe

*****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "sbox.h"

#include "flash.h"


/*-------------------------------------------------------
		タイマ1割り込み関数
-------------------------------------------------------*/
interrupt void int_led( void )
{
	volatile int	tmp;

	tmp = sbox_DiGet();
}

/*------------------------------------------------------------
	FLASHへ1ワード 書き込み
 -------------------------------------------------------------*/
int flash_write32( u_int adr, u_int data )
{
	u_int		set_adr;
	u_char		set_data;
	int			i;

	// 低いアドレスに下位データ
	for( i = 0; i < 4; i++ ) {
		set_adr = adr + i;
		set_data = (u_char)((data >> (i * 8)) & 0xFF);
		flash_WriteMode();
		*(volatile u_char *)set_adr = set_data;

		if( flash_WriteWait( set_adr, set_data ) != SBOX_OK ) {
			printf("write error  adr:0x%X   write:0x%02X  \n", set_adr, set_data );
			return( SBOX_ERROR );
		}
	}

	return( SBOX_OK );
}

/*-------------------------------------------------------
		main routine
-------------------------------------------------------*/
void main( void )
{
	sbox_Init();

	// 動作中表示のためLED点滅
	int_vect_enable( ETIMER1, int_led );
	clock_set( 2.0, TIMER_1 );

	// ROM消去
	printf("FLASH erase  wait \n");
	if( flash_Erase() != SBOX_OK ) {
		printf("ERROR : data check   adr:0x%08X  chk:0x%X  read:0x%X \n", 
						flash_error_info[0], 
						flash_error_info[1], flash_error_info[2] );
		clock_stop( TIMER_1 );
		exit( 0 );
	}

	clock_stop( TIMER_1 );

	printf("complete. \n");
}

