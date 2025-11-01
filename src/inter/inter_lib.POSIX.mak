# Copyright 2025 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2025-11-01
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# inter_lib.POSIX.mak
# Makefile for interpreter library (librefalab.a) creation
# result - lib/librefalab.a library
# Version for POSIX
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c .ref

CC = clang
CFLAGS = -pipe -Wall -O2 -DPOSIX
AR = ar
REFXAR2 = src/inter/refxar2
REFXCOMP2 = src/inter/refxcomp2
REFXEXT = src/inter/refxext
REFXFIO2 = src/inter/refxfio2
REFXHOF = src/inter/refxhof
REFXMO2 = src/inter/refxmo2
REFXPLATFORM = src/inter/refxplatformposix
REFLIB = lib/librefalab.a
S = src/inter

####### Files

SOURCES = \
    $(S)/d.c \
    $(S)/rfintf.c \
    $(S)/rfrun.c \
    $(S)/xar.c \
    $(S)/xcf.c \
    $(S)/xcomp.c \
    $(S)/xfio.c \
    $(S)/xio.c \
    $(S)/xjak.c \
    $(S)/xmo.c \
    $(S)/xrand.c \
    $(S)/xstor.c \
    $(S)/xsys.c \
    $(S)/xtime.c \
    $(S)/xtry.c 	

OBJECTS = \
    $(S)/d.o \
    $(S)/rfintf.o \
    $(S)/rfrun.o \
    $(S)/xar.o \
    $(S)/xar2.o \
    $(S)/xcf.o \
    $(S)/xcomp.o \
    $(S)/xcomp2.o \
    $(S)/xext.o \
    $(S)/xfio.o \
    $(S)/xfio2.o \
    $(S)/xhof.o \
    $(S)/xio.o \
    $(S)/xjak.o \
    $(S)/xmo.o \
    $(S)/xmo2.o \
    $(S)/xrand.o \
    $(S)/xplatform.o \
    $(S)/xstor.o \
    $(S)/xsys.o \
    $(S)/xtime.o \
    $(S)/xtry.o
    
####### Implicit rules

.ref.o:
	$(REFXAR2)
	$(REFXCOMP2)
	$(REFXEXT)
	$(REFXFIO2)
	$(REFXHOF)
	$(REFXMO2)
	$(REFXPLATFORM)
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

$(S)/xcomp.o: $(S)/xcomp.c

$(S)/xfio.o: $(S)/xfio.c

$(S)/xio.o: $(S)/xio.c

$(S)/xjak.o: $(S)/xjak.c

$(S)/xmo.o: $(S)/xmo.c

$(S)/xrand.o: $(S)/xrand.c

$(S)/xstor.o: $(S)/xstor.c

$(S)/xsys.o: $(S)/xsys.c

$(S)/xtime.o: $(S)/xtime.c

$(S)/xtry.o: $(S)/xtry.c

####### End of Makefile
