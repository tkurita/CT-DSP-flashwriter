cl6x -c -o2 -mv6700 -eo.o67 FlashWrite.c
lnk6x FlashWrite.o67 -x -o FlashWrite.out -m FlashWrite.map ..\..\..\lib\iramboot.cmd -l sboxdev.lib
copy FlashWrite.out ..\..\exe_flash
