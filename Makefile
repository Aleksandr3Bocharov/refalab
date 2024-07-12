#############################################################################
# Makefile
#       Makefile for building refal2 system
#       for Windows (Win95/98/ME/NT/2000/XP)
#
# Copyright (C) Refal-2 Team
# Author: Leonid Belous (aka BLF)
# E-mail: belous@ilt.kharkov.ua
# Date: 12.07.2024
#############################################################################

MAKE	=	mingw32-make

all:

# PHASE I:
# Make the compiler from refal-2 language, 
# result bin\refal2 executable module
	
	$(MAKE) -f src\comp\comp_win.mak
	src\comp\delbin.bat

# PHASE II:
# Make the interpreter from "jazyk sborki",  
# result lib/libr2.a - set of object modules.
# result lib/libr2_dbg.a - set of object modules (debug mode).
	
	$(MAKE) -f src\inter\inter_lib_win.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_lib_dbg_win.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_o_win.mak
	src\inter\copy_obj.bat
	src\inter\delbin.bat

#### end of Makefile
