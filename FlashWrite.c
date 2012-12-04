/*****************************************************************

		sBOX (DSK6713)  FLASH TOOL PROGRAM
		running on CCS

    	File Name   :   flashwrite.c
    	Ver.        :   1.0.0
    	Date        :   2006. 6. 5

	指定されたCOFFファイルを読み込んでROMイメージをSDRAMに作成
	作成したROMイメージをFLASH ROMへ書き込む
	CCS上からのみ実行可能

	データ幅 8bit
	80000000h ～	FLASH先頭に置くローダ部分
	80000400h ～	実行コード

    Copyright(c) 2006. Mtt Company DSP Lab. Kobe

*****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "sbox.h"

#include "flash.h"
#include "cofftype.h"


#define	USE_HEAD			DSK_SDRAM_HEAD			// SDRAM
#define FLASH_LOCK_OFFSET	0x400

#define LOADBUFSIZE			(0x1000)
#define NWORDS				(LOADBUFSIZE)

static char loadbuf[LOADBUFSIZE];

static COFF_OptHdr OPTHEADER = {
    (short)0,		// magic
    (short)0,		// vers
    0L,		 		// textSize
    0L,		    	// dataSize
    0L,		    	// bssSize
    0L,		    	// entryPoint
    0L,		    	// textAddr
    0L		    	// dataAddr
};

int		sect_cnt = 0;


/*---------------------------------------------------------------
   swap 2byte Swap the order of bytes in a short.
----------------------------------------------------------------*/
void swap2byte( short *addr )
{
    *addr = ((*addr & 0xFF) << 8) | ((*addr >> 8) & 0xFF);
}

/*---------------------------------------------------------------
   swap 4byte Swap the order of bytes in a int
----------------------------------------------------------------*/
void swap4byte( int *addr )
{
	int		word;

    word  = (*addr & 0xFFL) << 24;
    word |= (*addr & 0xFF00L) << 8;
    word |= (*addr & 0xFF0000L) >> 8;
    word |= (*addr & 0xFF000000L) >> 24;

    *addr = word;
}

/*---------------------------------------------------------------
---------------------------------------------------------------*/ 
COFF_Desc coffDescCreate( int *err )
{
	COFF_Desc desc;

    desc = (COFF_Desc)malloc( sizeof(COFF_DescObj) );
    if( desc == (COFF_Desc)NULL ){
		printf("ERROR : memory allocate \n");
    	return( NULL );
	}

	desc->byteSwapped = FALSE;
	desc->sectHdrs = NULL;

    return( desc );
}

/*---------------------------------------------------------------
---------------------------------------------------------------*/ 
void coffDescDelete( COFF_Desc desc )
{
    if( desc->sectHdrs != (COFF_SectHdr *)NULL ){
		free( (COFF_SectHdr *)desc->sectHdrs );
    }

	if( desc != (COFF_Desc)NULL )
    	free( (COFF_Desc)desc );
}

/*---------------------------------------------------------------
	File & Option header section Get
----------------------------------------------------------------*/
int getHeaders( COFF_Desc desc, FILE *filep )
{
	int		i;
	COFF_SectHdr	*sptr;
 
	// fixed size 22byte
    if( !fread( &desc->header, COFF_HEADER_SIZE, 1, filep) ){
		printf("ERROR : read file headder \n");
		return( SBOX_ERROR );
    }

	// データスワップの必要をチェック
	if( desc->header.magic != COFF_HDRMAGIC ){
		swap2byte( (short *)&desc->header.magic );
		if( desc->header.magic != COFF_HDRMAGIC ){
			printf("ERROR : magic number (0x%X/0x%X) \n", 
									COFF_HDRMAGIC, desc->header.magic);
			return( SBOX_ERROR );
		}
		desc->byteSwapped = TRUE; // unix

		swap2byte( (short *)&desc->header.nSects );
		swap4byte( &desc->header.date );
		swap4byte( &desc->header.symTabPtr );
		swap4byte( &desc->header.nSyms );
		swap2byte( (short *)&desc->header.optHdrSize );
		swap2byte( (short *)&desc->header.flags );
    }

	// DSP Endian 確認
	if(	desc->byteSwapped == TRUE ){ // unix
		if( (desc->header.flags & COFF_F_BIG) == 0 )
			desc->dspEndianSwap = 1; // swap
		else
			desc->dspEndianSwap = 0; // no swap
	}
	else{
		if( (desc->header.flags & COFF_F_BIG) == 0 )
			desc->dspEndianSwap = 0; // no swap
		else
			desc->dspEndianSwap = 1; // swap
	}

    // オプションヘッダ
    if( desc->header.optHdrSize == sizeof(COFF_OptHdr) ){
		if( fread( &desc->optHeader, desc->header.optHdrSize, 1, filep) != 1) {
			printf("ERROR : read option headdre \n");
			return( SBOX_ERROR );
		}
		if(desc->byteSwapped ){
			swap2byte( &desc->optHeader.magic );
			swap2byte( &desc->optHeader.vers );
			swap4byte( &desc->optHeader.textSize );
			swap4byte( &desc->optHeader.dataSize );
			swap4byte( &desc->optHeader.bssSize );
			swap4byte( &desc->optHeader.entryPoint );
			swap4byte( &desc->optHeader.textAddr );
			swap4byte( &desc->optHeader.dataAddr );
		}
    }
    else{
		desc->optHeader = OPTHEADER;
		if( ( desc->header.optHdrSize != 0 ) &&
					( fseek( filep, (int)desc->header.optHdrSize, 1 ) == -1 ) ){
			printf("ERROR : optionheadder size (%d) \n", 
										(int)desc->header.optHdrSize);
			return( SBOX_ERROR );
		}
    }

    if( desc->sectHdrs != NULL ){
		free(desc->sectHdrs);
    }

    desc->sectHdrs = (COFF_SectHdr *)malloc( desc->header.nSects * sizeof(COFF_SectHdr) );
    if( (desc->sectHdrs == NULL) ||
		(fread(desc->sectHdrs, sizeof(COFF_SectHdr), desc->header.nSects, filep) !=
		 desc->header.nSects ) ){
		printf("ERROR : read section headder \n");
		return( SBOX_ERROR );
    }

    if( desc->byteSwapped ){
		for( i=0, sptr=desc->sectHdrs; i++<desc->header.nSects; sptr++ ){
			swap4byte( (int *)&sptr->physAddr );
			swap4byte( (int *)&sptr->virtAddr );
			swap4byte( &sptr->size );
			swap4byte( &sptr->dataPtr );
			swap4byte( (int *)&sptr->relocPtr );
			swap4byte( (int *)&sptr->linePtr );
			swap4byte( (int *)&sptr->nRelocs );
			swap4byte( (int *)&sptr->nLines );
			swap4byte( (int *)&sptr->flags );
		}
    }

	return( SBOX_OK );
}

/*---------------------------------------------------------------
	write Section datas to INRAM
----------------------------------------------------------------*/
int writeSect( COFF_Desc desc, FILE *filep, COFF_SectHdr *sptr, u_int *ram_adr )
{
	int		total, nwords, *p;
	int 	i, flag, wordlen;
	u_int	adr, data;

	// loader
	if( sptr->physAddr < FLASH_LOCK_OFFSET ){
		flag = 0;
		adr = USE_HEAD + sptr->physAddr;
	}
	// execute code section
	else{
		flag = 1;
		adr = *ram_adr;
	}

	printf("%s : phys:%X  virt:%X  sdram:%X  file:%X  size:%X \n", 
			sptr->name, sptr->physAddr, sptr->virtAddr, adr, sptr->dataPtr, sptr->size );

	// 転送するセクションへ移動
	if( fseek( filep, sptr->dataPtr, SEEK_SET) ){
		printf("ERROR : file seek (section) \n");
		return( SBOX_ERROR );
	}

	// ロ－ダ－以外のセクションの場合、サイズと再配置先を先頭に置く
	if( flag ){
		data = (sptr->size);			// バイト数
		*(volatile u_int *)adr = data;
		adr += 4;

		data = sptr->physAddr; 
		if( data > DSK_FLASH_HEAD )		data -= DSK_FLASH_HEAD;
		*(volatile u_int *)adr = data;
		adr += 4;
	}

	total = (sptr->size);

    for( ; total > 0; total -= nwords ){
		nwords = MIN( total, NWORDS );

		if( fread( loadbuf, nwords, 1, filep ) != 1 ){
			printf("ERROR : file read (section) \n");
			return( SBOX_ERROR );
		}

		wordlen = nwords / 4;
		if( (nwords % 4) != 0 )
			wordlen += 1;

		p = (int *)loadbuf;
		for( i=0 ; i<wordlen ; i++ ){
			// DSPプログラムのエンディアン対応
			if( desc->dspEndianSwap == 0 ) {
				*p = SWAP_NO( *p );
			}
			else {
				*p = SWAP( *p );
			}

			*(volatile int *)adr = *p;
			adr += 4;
			p++;
		}
    }

	if( flag == 1 ) {
		*ram_adr  = adr;
		sect_cnt++;
	}

    return( SBOX_OK );
}

/*---------------------------------------------------------------
	ROMイメージをSDRAMに作成
----------------------------------------------------------------*/
int writeRomImage( COFF_Desc desc, FILE *filep, u_int *lenp )
{
	COFF_SectHdr	*sptr;
	u_int	ram_adr;
	int		s, size, cnt;
	int		rc;

	ram_adr = USE_HEAD + FLASH_LOCK_OFFSET + 4;			// 先頭はセクション数

	for( s=0, sptr=desc->sectHdrs; s<desc->header.nSects; s++, sptr++ ) {
		/*
		 * Ignore empty sections or sections whose flags indicate the
		 * section is not to be loaded.  Note that the .cinit section,
		 * although it has the COFF_S_COPY flag set, must be loaded.   
		 */
		if( sptr->dataPtr == 0L ||
			sptr->size == 0L ||
			( sptr->flags & COFF_S_DSECT ) ||
			( sptr->flags & COFF_S_COPY ) && strcmp(sptr->name, ".cinit") ||
			( sptr->flags & COFF_S_NOLOAD ) ){
			continue;
		}

		rc = writeSect(desc, filep, sptr, &ram_adr );
		if( rc != SBOX_OK ) {
			return( rc );
		}
    }

	size = ram_adr - (USE_HEAD + FLASH_LOCK_OFFSET);		// byte
	cnt = size / 4;
	if( (size % 4) != 0 )		cnt++;
	*lenp = cnt;											// word
	*(volatile int *)(USE_HEAD + FLASH_LOCK_OFFSET) = sect_cnt;	// 先頭にセクション数

	// ROMイメージのサイズ確認
	if( size > (DSK_FLASH_TAIL - DSK_FLASH_HEAD + 1) ) {
		printf("ERROR : data is over the ROM size (0x%X) \n", size );
		return( SBOX_ERROR );
	}

	// データの後ろを少しクリア（ROMには書かない）
	for( s=0; s<16; s++ ) {
		*(volatile int *)ram_adr = 0;
		ram_adr += 4;
	}

    return( SBOX_OK );
}

/*--------------------------------------------------------------
	FLASHへ書き込むROMイメージを作成
--------------------------------------------------------------*/
int create_rom_image( const char *coffname, u_int *size_ptr )
{
	COFF_Desc	loader;
	FILE		*fp;
	int			rc;

	// COFFファイル オープン
	if( ( fp = fopen( coffname, "rb" ) ) == NULL ) {
		printf("ERROR : file not open [%s] \n", coffname );
		return( SBOX_ERROR );
	}

	// COFF用メモリ 確保
	loader = coffDescCreate( &rc );
	if( loader == (COFF_Desc)NULL ) {
		printf("ERROR : memory allocate for COFF \n");
		fclose( fp );
		return( SBOX_ERROR );
	}

	// セクションヘッダ取得
	rc = getHeaders( loader, fp );
	if( rc != SBOX_OK ) {
		printf("ERROR : get COFF header \n");
		fclose( fp );
		coffDescDelete( loader );
	}

	// セクションイメージ SDRAM書込み
	rc = writeRomImage( loader, fp, size_ptr );

	// COFFファイル クローズ
	fclose( fp );

	// COFF用メモリ領域 開放
	coffDescDelete( loader );

	return( rc );
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
			return( SBOX_ERROR );
		}
	}

	return( SBOX_OK );
}

/*------------------------------------------------------------
	FLASH書き込み
 -------------------------------------------------------------*/
int write_section( int head, u_int text_len )
{
	volatile u_int	*ptr;
	u_int		i,len;
	u_int		adr;
	u_int		data;

	// ローダ書き込み 0x400byte固定
	adr   = DSK_FLASH_HEAD;
	ptr = (volatile u_int *)head;
	len = FLASH_LOCK_OFFSET / 4;

	for( i=0; i<len; i++ ) {
		data = *ptr;
		if( flash_write32( adr, data ) != SBOX_OK ) {
			return( SBOX_ERROR );
		}
		ptr++;
		adr += 4;
	}

	// コード書き込み
	adr = DSK_FLASH_HEAD + FLASH_LOCK_OFFSET;
	ptr = (volatile u_int *)(USE_HEAD + FLASH_LOCK_OFFSET);
	len = text_len;

	for( i=0; i<len; i++ ) {
		data = *ptr;
		if( flash_write32( adr, data ) != SBOX_OK ) {
			return( SBOX_ERROR );
		}
		ptr++;
		adr += 4;
	}

	// 終了コード書き込み
	data = 0;
	if( flash_write32( adr, data ) != SBOX_OK ) {
		return( SBOX_ERROR );
	}

	return( SBOX_OK );
}

#if 0
/*-------------------------------------------------------
		コピーローダ動作確認（debug用）
-------------------------------------------------------*/
void relocate_check( void )
{
	unsigned int	rom_adr;
	int		sect, adr, len, odd;
	int		i;

	rom_adr = 0x90000400;
	sect = *(volatile int *)rom_adr;
	rom_adr += 4;

	for( i = 0; i < sect; i++ ) {
		len = *(volatile int *)rom_adr / 4;
		odd = *(volatile int *)rom_adr % 4;
		adr = *(volatile int *)(rom_adr + 4);

		printf("[%d] adr:0x%08X -> 0x%08X   size:0x%X \n", 
									i+1, rom_adr, adr, (len*4+odd) );

		rom_adr += 8;

		for( ; len > 0; len-- ) {
			rom_adr += 4;
		}

		for( ; odd > 0; odd-- ) {
			rom_adr++;
		}
	}
}
#endif

/*-------------------------------------------------------
		タイマ1割り込み関数
-------------------------------------------------------*/
interrupt void int_led( void )
{
	volatile int	tmp;

	tmp = sbox_DiGet();
	tmp = sbox_DiGet();
	tmp = sbox_DiGet();
	tmp = sbox_DiGet();
	tmp = sbox_DiGet();
}

/*-------------------------------------------------------
		main routine
-------------------------------------------------------*/
void main( void )
{
	//char	fname[256];
	const char *fname = "../../CT-DSP/flash/CT-DSP-flash.out";
	u_int	text_size = 0;

	sbox_Init();

	for( text_size=0; text_size<FLASH_LOCK_OFFSET; text_size+=4 ) {
		*(volatile int *)(USE_HEAD + text_size) = 0;
	}

	// DSPプログラムファイル名入力
	//printf("input DSP filename ... ");
	//gets( fname );

	// 動作中表示のためLED点滅
	int_vect_enable( ETIMER1, int_led );
	clock_set( 1.0, TIMER_1 );

	// ROMイメージ作成
	if( create_rom_image( fname, &text_size ) != SBOX_OK ) {
		clock_stop( TIMER_1 );
		exit( 0 );
	}

	// ROM消去
	printf("FLASH erase  wait \n");
	if( flash_Erase() != SBOX_OK ) {
		printf("ERROR : data check   adr:0x%08X  chk:0x%X  read:0x%X \n", 
						flash_error_info[0], 
						flash_error_info[1], flash_error_info[2] );
		clock_stop( TIMER_1 );
		exit( 0 );
	}

	// write code
	printf("FLASH writing \n");
	if( write_section( USE_HEAD, text_size ) != SBOX_OK ) {
		printf("ERROR : data check   adr:0x%08X  chk:0x%X  read:0x%X \n", 
						flash_error_info[0], 
						flash_error_info[1], flash_error_info[2] );
		clock_stop( TIMER_1 );
		exit( 0 );
	}

//	relocate_check();		// debug用

	clock_stop( TIMER_1 );

	printf("complete.   text size = 0x%X byte \n", text_size*4 );
	puts("Restart sBox for Slow CT\n");
}

