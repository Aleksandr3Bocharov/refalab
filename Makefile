# Copyright 2025 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2025-04-14
# https://github.com/Aleksandr3Bocharov/RefalAB

#############################################################################
# Makefile
# Makefile for building RefalAB system
# Version for Windows
#############################################################################

MAKE = mingw32-make

all:

# PHASE I:
# Make the compiler from RefalAB language
# result - bin\RefalAB executable module
# result - bin\RefalAB_dbg executable module (debug mode)

	if not exist bin mkdir bin
	src\comp\delbin.bat
	$(MAKE) -f src\comp\comp.Windows.mak
	src\comp\delbin.bat
	$(MAKE) -f src\comp\comp_dbg.Windows.mak
	src\comp\delbin.bat

# PHASE II:
# Make the interpreter from "jazyk sborki" 
# result - lib\libRefalAB.a library
# result - lib\libRefalAB_dbg.a library (debug mode)
# result - lib\mainrf.o lib\rfdbg.o

	if not exist lib mkdir lib
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_lib.Windows.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_lib_dbg.Windows.mak
	src\inter\delbin.bat
	$(MAKE) -f src\inter\inter_o.Windows.mak
	src\inter\copy_obj.bat
	src\inter\delbin.bat

#### End of Makefile
