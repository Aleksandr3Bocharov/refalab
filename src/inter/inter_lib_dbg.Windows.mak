# Copyright 2025 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2025-10-14
# https://github.com/Aleksandr3Bocharov/RefalAB

#############################################################################
# inter_lib_dbg.Windows.mak
# Makefile for interpreter library (libRefalAB_dbg.a) (debug mode) creation
# result - lib\libRefalAB_dbg.a library
# Version for Windows
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c .ref

CC = clang
CFLAGS = -pipe -Wall -O2 -Dmdebug
AR = ar
REFXCV = src\inter\refxcv.bat
REFEXT = src\inter\refext.bat
REFPLATFORM = src\inter\refplatform.bat
REFLIB = lib\libRefalAB_dbg.a
S = src\inter

####### Files

SOURCES = \
    $(S)\d.c \
    $(S)\rfintf.c \
    $(S)\rfrun.c \
    $(S)\xar.c \
    $(S)\xcf.c \
    $(S)\xfio.c \
    $(S)\xgcd.c \
    $(S)\xio.c \
    $(S)\xjak.c \
    $(S)\xmo.c \
    $(S)\xrand.c \
    $(S)\xsys.c \
    $(S)\xtime.c \
    $(S)\xtry.c

OBJECTS = \
    $(S)\d.o \
    $(S)\rfintf.o \
    $(S)\rfrun.o \
    $(S)\xar.o \
    $(S)\xcf.o \
    $(S)\xcv.o \
    $(S)\xext.o \
    $(S)\xfio.o \
    $(S)\xgcd.o \
    $(S)\xio.o \
    $(S)\xjak.o \
    $(S)\xmo.o \
    $(S)\xrand.o \
    $(S)\xplatformwin.o \
    $(S)\xsys.o \
    $(S)\xtime.o \
    $(S)\xtry.o

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

$(S)\d.o: $(S)\d.c

$(S)\rfintf.o: $(S)\rfintf.c

$(S)\rfrun.o: $(S)\rfrun.c

$(S)\xar.o: $(S)\xar.c

$(S)\xcf.o: $(S)\xcf.c

$(S)\xfio.o: $(S)\xfio.c

$(S)\xgcd.o: $(S)\xgcd.c

$(S)\xio.o: $(S)\xio.c

$(S)\xjak.o: $(S)\xjak.c

$(S)\xmo.o: $(S)\xmo.c

$(S)\xrand.o: $(S)\xrand.c

$(S)\xsys.o: $(S)\xsys.c

$(S)\xtime.o: $(S)\xtime.c

$(S)\xtry.o: $(S)\xtry.c

####### End of Makefile
