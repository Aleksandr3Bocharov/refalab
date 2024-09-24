#############################################################################
# Makefile
#       Makefile for building RefalAB system
#       for Windows
#
# Copyright(C) Aleksandr Bocharov
# Author : Aleksandr Bocharov
# Date: 24.09.2024
#############################################################################

MAKE	=	mingw32-make

all:

# PHASE I:
# Make the compiler from RefalAB language, 
# result bin\RefalAB executable module.
# result bin\RefalAB_dbg executable module (debug mode).
	
	$(MAKE) -f src\comp\comp_win.mak
	src\comp\delbin.bat
	$(MAKE) -f src\comp\comp_dbg_win.mak
	src\comp\delbin.bat

# PHASE II:
# Make the interpreter from "jazyk sborki",  
# result lib\libRefalAB.a - set of object modules.
# result lib\libRefalAB_dbg.a - set of object modules (debug mode).
	
	$(MAKE) -f src\inter\inter_lib_win.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_lib_dbg_win.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_o_win.mak
	src\inter\copy_obj.bat
	src\inter\delbin.bat

#### end of Makefile
