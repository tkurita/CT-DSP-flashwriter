#
#		Title	    : sBOX program makefile
#		Environment : Windows
#       Compiler    : TI-C  Rel.4.20
#       Version     : 1.0.0
#       Date        : 2006. 6. 9
#
#	Copyright(c) 2006  MTT Company DSP Lab. Kobe
#

SAMPLE		= FlashWrite

EXEDIR		= ..\..\exe_flash

C6XCMD		= ..\..\..\lib\iramboot.cmd
C6XLIB		= -l sboxdev.lib

#
C6XCL		= cl6x
C6XLD		= lnk6x
CC6XFLAGS   = -c -o2 -mv6700 -eo.o67
C6XLDFLAGS  = -x -o $*.out -m $*.map
#
.SUFFIXES:	.out .o67

#
.c.o67:
	$(C6XCL) $(CC6XFLAGS) $<
.o67.out:
        $(C6XLD) $< $(C6XLDFLAGS) $(C6XCMD) $(C6XLIB)
		copy $@ $(EXEDIR)

all:	$(SAMPLE).out

#
$(SAMPLE).out: $(SAMPLE).o67

#
clean:
	del *.o67
	del *.map
	del *.bak

