#############################################################################
# Makefile
#       Makefile for building refal2 system
#       for Windows (Win95/98/ME/NT/2000/XP)
#
# Copyright (C) Refal-2 Team
# Author: Leonid Belous (aka BLF)
# E-mail: belous@ilt.kharkov.ua
# Date: 28.05.2005
#############################################################################

MAKE	=	mingw32-make
DEL	=	del

all:

# PHASE I:
# Make the compiler from refal-2 language, 
# result bin\refal2 executable module
	
	$(MAKE) -f src\comp\comp.mak

# PHASE II:
# Make the interpreter from "jazyk sborki",  
# result lib/librefal2.a - set of object modules.
# Empty  lib/libr2user.a also created.
	
	$(MAKE) -f src\inter\inter.mak

#### end of Makefile
