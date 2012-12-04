/*------------------------------------------------------------
			sBOX (DSK6713)
			FLASH access tool headder file

			File Name	:	flash.h
			Ver.		:	1.0.0
			Date		:	2006. 5. 15

			AMD  Am29LV400B
			8bitアクセス

		Copyright(C) 2006. MTT Company DSP Lab. Kobe
-------------------------------------------------------------*/
#ifndef _FLASH_H_
#define _FLASH_H_

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;

//--------------------- DSK6713 memory -------------------
#define	DSK_SDRAM_HEAD			0x80000000
#define	DSK_FLASH_HEAD			0x90000000
#define	DSK_FLASH_TAIL			0x9003FFFF


//------------------- error code  -------------------
#define	PROCCESS_FAIL			-201


//------------------- prototype  -------------------
int		flash_WriteWait( u_int, u_char );
int		flash_Erase( void );
void	flash_WriteMode( void );

extern u_int	flash_error_info[];


#endif /* _FLASH_H_ */

