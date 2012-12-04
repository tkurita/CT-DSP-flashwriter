/*------------------------------------------------------------
			sBOX (DSK6713)
			FLASH access tool source file

			File Name	:	faccs.c
			Ver.		:	1.0.0
			Date		:	2006. 5. 15

			AMD  Am29LV400B
			8bitアクセス
			イレーズ、ライトのタイムアウトにタイマ0を使用

		Copyright(C) 2006. MTT Company DSP Lab. Kobe
-------------------------------------------------------------*/
#include "sbox.h"
#include "flash.h"

#define	TICK_PER_SEC			(DSP_CLOCK_RATE / 4.0)		// for CLOCK mode
#define	FLASH_SECT_NUM			11
#define	WAIT_ERASE_SECT_SEC		15
#define	WAIT_ERASE_SEC			(WAIT_ERASE_SECT_SEC * FLASH_SECT_NUM)
#define	WAIT_WRITE_SEC			5

u_int	flash_error_info[3] = { 0, 0, 0 };

extern volatile int		sbox_sys_error;


/*---------------------------------------------------------------*/
static int flash_sleep( int msec )
{
	volatile u_int	*reg_ctrl, *reg_freq, *reg_cnt;
	u_int	check_cnt, bak_ctrl, bak_freq;
	float	offset;

	reg_ctrl = (volatile u_int *)DSP_TIMER0_CTRL;
	reg_freq = (volatile u_int *)DSP_TIMER0_FREQ;
	reg_cnt  = (volatile u_int *)DSP_TIMER0_CNT;

	offset    = TICK_PER_SEC * 0.001;				// (225MHz / 4) / 1000
	check_cnt = (u_int)(offset * (float)msec);
	bak_ctrl  = *reg_ctrl;
	bak_freq  = *reg_freq;

	*reg_freq = 0xFFFFFFFF;
	*reg_ctrl = TIMER_SET_MODE | TIMER_HOLD;

	*reg_ctrl |= TIMER_GO;

	while( check_cnt >= *reg_cnt );

	*reg_ctrl = bak_ctrl;
	*reg_freq = bak_freq;

	return( SBOX_OK );
}

/*------------------------------------------------------------------------
	flash read data check wait 
------------------------------------------------------------------------*/
static int flash_Wait( u_int adr, u_char chk )
{
  int		chk_time;

	chk_time = WAIT_ERASE_SEC * 1000;

	while( 1 ){
		if( *(volatile u_char *)adr == chk )		break;

		if( --chk_time < 0 ) {
			flash_error_info[0] = adr;
			flash_error_info[1] = (u_int)chk;
			flash_error_info[2] = (u_int)(*(volatile u_char *)adr);
			sbox_sys_error = PROCCESS_FAIL;
			return( SBOX_ERROR );
		}

		flash_sleep( 1 );
	}

	return( SBOX_OK );
}

/*------------------------------------------------------------------------
	flash write, check read
------------------------------------------------------------------------*/
int flash_WriteWait( u_int adr, u_char chk )
{
  int		sec;

	*(volatile u_int *)DSP_TIMER0_FREQ = 0xFFFFFFFF;
	*(volatile u_int *)DSP_TIMER0_CTRL = TIMER_SET_MODE | TIMER_HOLD;
	*(volatile u_int *)DSP_TIMER0_CTRL |= TIMER_GO;
	*(volatile u_int *)DSP_TIMER0_CNT = 0;
	sec = 0;

	// wait complete
	while( 1 ){
		if( *(volatile u_char *)adr == chk )		break;

		if( *DSP_TIMER0_CNT >= TICK_PER_SEC ) {
			sec++;
			*DSP_TIMER0_CNT = 0;
		}

		if( sec >= WAIT_WRITE_SEC ) {
			flash_error_info[0] = adr;
			flash_error_info[1] = (u_int)chk;
			flash_error_info[2] = (u_int)(*(volatile u_char *)adr);
			sbox_sys_error = PROCCESS_FAIL;
			return( SBOX_ERROR );
		}
	}

	*(volatile u_int *)DSP_TIMER0_CTRL &= ~TIMER_GO;

	return( SBOX_OK );
}

/*------------------------------------------------------------------------
	flash chip erase
------------------------------------------------------------------------*/
int flash_Erase( void )
{
  u_int		adr;

	adr = DSK_FLASH_HEAD;

	*(volatile u_char *)(adr + 0x555) = 0xAA;
	*(volatile u_char *)(adr + 0x2AA) = 0x55;
	*(volatile u_char *)(adr + 0x555) = 0x80;
	*(volatile u_char *)(adr + 0x555) = 0xAA;
	*(volatile u_char *)(adr + 0x2AA) = 0x55;
	*(volatile u_char *)(adr + 0x555) = 0x10;

	adr = DSK_FLASH_TAIL;
	if( flash_Wait( adr, 0xFF ) != SBOX_OK ) {
		sbox_sys_error = PROCCESS_FAIL;
		return( SBOX_ERROR );
	}

	return( SBOX_OK );
}

/*------------------------------------------------------------------------
	write mode
------------------------------------------------------------------------*/
void flash_WriteMode( void )
{
  u_int		adr;

	adr = DSK_FLASH_HEAD;

	*(volatile u_char *)(adr + 0x555) = 0xAA;
	*(volatile u_char *)(adr + 0x2AA) = 0x55;
	*(volatile u_char *)(adr + 0x555) = 0xA0;
}

