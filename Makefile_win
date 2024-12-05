# Copyright 2024 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2024-11-23
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
$(MAKE) -f src\comp\comp_win.mak
src\comp\delbin.bat
$(MAKE) -f src\comp\comp_dbg_win.mak
src\comp\delbin.bat

# PHASE II:
# Make the interpreter from "jazyk sborki" 
# result - lib\libRefalAB.a library
# result - lib\libRefalAB_dbg.a library (debug mode)
# result - lib\mainrf.o lib\rfdbg.o

if not exist lib mkdir lib
$(MAKE) -f src\inter\inter_lib_win.mak
src\inter\delbin.bat
$(MAKE) -f src\inter\inter_lib_dbg_win.mak
src\inter\delbin.bat
$(MAKE) -f src\inter\inter_o_win.mak
src\inter\copy_obj.bat
src\inter\delbin.bat

#### End of Makefile
