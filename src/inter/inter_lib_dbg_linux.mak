# Copyright 2025 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2025-03-30
# https://github.com/Aleksandr3Bocharov/RefalAB

#############################################################################
# inter_lib_dbg_linux.mak
# Makefile for interpreter library (libRefalAB_dbg.a) (debug mode) creation
# result - lib/libRefalAB_dbg.a library
# Version for Linux (UNIX, FreeBSD)
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c .ref

CC = clang
CFLAGS = -pipe -Wall -O2 -Dmdebug -DUNIX
AR = ar
REFXCV = src/inter/refxcv
REFEXT = src/inter/refext
REFPLATFORM = src/inter/refplatform
REFLIB = lib/libRefalAB_dbg.a
S = src/inter

####### Files

SOURCES = \
    $(S)/d.c \
    $(S)/rfintf.c \
    $(S)/rfrun.c \
    $(S)/xar.c \
    $(S)/xcf.c \
    $(S)/xgcd.c \
    $(S)/xjak.c \
    $(S)/xmo.c \
    $(S)/xrename.c \
    $(S)/xsys.c \
    $(S)/xtime.c \
    $(S)/xtry.c \
    $(S)/xvv.c \
    $(S)/xvv5.c 	

OBJECTS = \
    $(S)/d.o \
    $(S)/rfintf.o \
    $(S)/rfrun.o \
    $(S)/xar.o \
    $(S)/xcf.o \
    $(S)/xcv.o \
    $(S)/xext.o \
    $(S)/xgcd.o \
    $(S)/xjak.o \
    $(S)/xmo.o \
    $(S)/xplatform.o \
    $(S)/xrename.o \
    $(S)/xsys.o \
    $(S)/xtime.o \
    $(S)/xtry.o \
    $(S)/xvv.o \
    $(S)/xvv5.o

####### Implicit rules

.ref.o: 
	$(REFXCV)  
	$(REFEXT) 
	$(REFPLATFORM)
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

####### Build rules

all: $(REFLIB)

$(REFLIB): $(OBJECTS)
	$(AR) rs $(REFLIB) $(OBJECTS) 

####### Dependences

$(S)/d.o: $(S)/d.c

$(S)/rfintf.o: $(S)/rfintf.c

$(S)/rfrun.o: $(S)/rfrun.c

$(S)/xar.o: $(S)/xar.c

$(S)/xcf.o: $(S)/xcf.c

$(S)/xcoden.o: $(S)/xcoden.c

$(S)/xgcd.o: $(S)/xgcd.c

$(S)/xjak.o: $(S)/xjak.c

$(S)/xmo.o: $(S)/xmo.c

$(S)/xrename.o: $(S)/xrename.c

$(S)/xsys.o: $(S)/xsys.c

$(S)/xtime.o: $(S)/xtime.c

$(S)/xtry.o: $(S)/xtry.c

$(S)/xvv.o: $(S)/xvv.c

$(S)/xvv5.o: $(S)/xvv5.c

####### End of Makefile
