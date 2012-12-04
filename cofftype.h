/*------------------------------------------------------------
			sBOX (DSK6713)
			FLASH access tool headder file

			File Name	:	cofftype.h
			Ver.		:	1.0.0
			Date		:	2006. 5. 15

			C6713 COFFフォーマット

		Copyright(C) 2006. MTT Company DSP Lab. Kobe
-------------------------------------------------------------*/
#ifndef _EDSP_COFF_H_
#define _EDSP_COFF_H_

#ifndef TRUE
#define	TRUE		1
#endif

#ifndef FALSE
#define	FALSE		0
#endif

#define COFF_HDRMAGIC	(0x99)
#define COFF_OPTMAGIC	(0x108)
#define COFF_NAMELEN	(8)
#define COFF_FILELEN	(14)
#define COFF_DIMNUM		(4)
#define MAXSYMLEN		(80)


#define	COFF_HEADER_SIZE	22		// 4byte バインダリ時に固定サイズで読み込み
#define SYM_STRUCT_SIZE		18

typedef struct COFF_FileHdr {
    u_short	coffid;			// Coff version ID
    u_short	nSects;			// number of sections
    int		date;			// time & date stamp
    int		symTabPtr;		// file pointer to symtab
    int		nSyms;			// number of symtab entries
    u_short	optHdrSize;		// sizeof(optional hdr)
    u_short	flags;			// flags
    u_short	magic;			// magic number
} COFF_FileHdr;

#define COFF_F_RELFLG (0x0001)  // relocation info stripped from file
#define COFF_F_EXEC   (0x0002)  // file is executable (no unresolved refs)
#define COFF_F_LNNO   (0x0004)	// line nunbers stripped from file
#define COFF_F_LSYMS  (0x0008)	// local symbols stripped from file
#define COFF_F_SWABD  (0x0040)	// bytes swabbed (in names)
#define COFF_F_AR16WR (0x0080)	// byte ordering of an AR16WR (PDP-11)
#define COFF_F_AR32WR (0x0100)	// byte ordering of an AR32WR (vax)
#define COFF_F_BIG    (0x0200)	// byte ordering of an AR16W  (3B, maxi)
#define COFF_F_PATCH  (0x0400)	// contains "patch" list in optional header

typedef struct COFF_OptHdr {
    short	magic;				// magic number
    short	vers;				// version stamp
    int		textSize;			// text size in bytes, padded to FW
    int		dataSize;			// initialized data "  "
    int		bssSize;			// uninitialized data "   "
    int		entryPoint;			// entry point
    int		textAddr;			// base of text used for this file
    int		dataAddr;			// base of data used for this file
} COFF_OptHdr;

typedef struct COFF_SectHdr {
    char	name[COFF_NAMELEN];	// section name
    u_int	physAddr;			// physical address
    u_int	virtAddr;			// virtual address
    int		size;				// section size
    int		dataPtr;			// file ptr to raw data for section
    int		relocPtr;			// file ptr to relocation
    int		linePtr;			// file ptr to line numbers
    u_int	nRelocs;			// number of relocation entries
    u_int	nLines;				// number of line number entries
    u_int	flags;				// flags
    u_short	dummy;				// reserved byte
    u_short	pageNum;			// memory page id
} COFF_SectHdr;

// section header flags
#define COFF_S_REG    (0x0000)	// regular
#define COFF_S_DSECT  (0x0001)	// dummy
#define COFF_S_NOLOAD (0x0002)	// noload
#define COFF_S_GROUP  (0x0004)	// grouped
#define COFF_S_PAD    (0x0008)	// padding
#define COFF_S_COPY   (0x0010)	// copy    : used for C init tables - 
								//			 not allocated, relocated,
								//			 loaded;  reloc & lineno
								//			 entries processed normally
#define COFF_S_TEXT   (0x0020)	// section contains text only
#define COFF_S_DATA   (0x0040)	// section contains data only
#define COFF_S_BSS    (0x0080)	// section contains bss  only
#define COFF_S_ALIGN  (0x0100)	// align flag used by old version assemblers

typedef union COFF_NameDesc {
    char	name[COFF_NAMELEN];
    struct {
		int		immedFlag;
		int		ptr;
    } stringPtr;
} COFF_NameDesc;

typedef struct COFF_Symbol {
    COFF_NameDesc	nameDesc;	
    int				value;		// value of symbol
    short			sectNum;	// section number
    u_short			type;		// type and derived type
    char			symclass;	// storage class
    char			numAux;		// number of aux. entries
} COFF_Symbol;

// storage classes
#define COFF_C_EFCN     (-1)		// physical end of function
#define COFF_C_NULL		(0)
#define COFF_C_AUTO		(1)		// automatic variable
#define COFF_C_EXT		(2)		// external symbol
#define COFF_C_STA		(3)		// static
#define COFF_C_REG		(4)		// register variable
#define COFF_C_EXTDEF	(5)		// external definition
#define COFF_C_LABEL	(6)		// label
#define COFF_C_ULABEL	(7)		// undefined label
#define COFF_C_MOS		(8)		// member of structure
#define COFF_C_ARG		(9)		// function argument
#define COFF_C_STRTAG	(10)	// structure tag
#define COFF_C_MOU		(11)	// member of union
#define COFF_C_UNTAG	(12)	// union tag
#define COFF_C_TPDEF	(13)	// type definition
#define COFF_C_USTATIC	(14)	// undefined static
#define COFF_C_ENTAG	(15)	// enumeration tag
#define COFF_C_MOE		(16)	// member of enumeration
#define COFF_C_REGPARM	(17)	// register parameter
#define COFF_C_FIELD	(18)	// bit field
#define COFF_C_BLOCK	(100)	// ".bb" or ".eb"
#define COFF_C_FCN		(101)	// ".bf" or ".ef"
#define COFF_C_EOS		(102)	// end of structure
#define COFF_C_FILE		(103)	// file name
#define COFF_C_LINE		(104)	// dummy sclass for line number entry

typedef struct {
    COFF_FileHdr	header;
    COFF_OptHdr		optHeader;
    COFF_SectHdr	*sectHdrs;
    int				byteSwapped;
	int				dspEndianSwap;
	int				ver;
} COFF_DescObj;

typedef COFF_DescObj *COFF_Desc;

// section numbers
#define COFF_N_DEBUG  (-2)
#define COFF_N_ABS    (-1)
#define COFF_N_UNDEF  (0)
#define COFF_N_TEXT   (1)
#define COFF_N_DATA   (2)
#define COFF_N_BSS    (3)


// DSP Program no swap
#define SWAP_NO(a)	(a)
// DSP Program  swap
#define SWAP(a)    (((a >> 24) & 0x000000ffL) | \
					((a >>  8) & 0x0000ff00L) | \
					((a <<  8) & 0x00ff0000L) | \
					((a << 24) & 0xff000000L))

#define MIN(a,b) ((a)<(b)?(a):(b))


#define SYM_INFO_FILE		".sdt"

// シンボル情報リスト
typedef	struct {
	u_int		n_value;
	int			nextptr;
	char		n_name[2];	//  [2] is dummy. real chars are copied here.
} SymListInfo;


#endif

