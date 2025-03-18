# Copyright 2025 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2025-03-18
# https://github.com/Aleksandr3Bocharov/RefalAB

#############################################################################
# comp_linux.mak
# Makefile for building of the compiler from RefalAB language 
# result - bin/RefalAB executable module
# Version for Linux (UNIX, FreeBSD)
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c 

CC = clang
CFLAGS = -pipe -Wall -O2 -DUNIX
LINK = clang

S = src/comp
BIN_DIR = bin

####### Files

SOURCES = \
    $(S)/ccst.c \
    $(S)/cerr.c \
    $(S)/cgop.c \
    $(S)/cj.c \
    $(S)/clu.c \
    $(S)/cs.c \
    $(S)/refal.c

OBJECTS = \
    $(S)/ccst.o \
    $(S)/cerr.o \
    $(S)/cgop.o \
    $(S)/cj.o \
    $(S)/clu.o \
    $(S)/cs.o \
    $(S)/refal.o

TARGET = RefalAB

####### Implicit rules

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(BIN_DIR)/$(TARGET) $(OBJECTS) 

####### Dependences

$(S)/ccst.o: $(S)/ccst.c

$(S)/cerr.o: $(S)/cerr.c

$(S)/cgop.o: $(S)/cgop.c

$(S)/cj.o: $(S)/cj.c

$(S)/clu.o: $(S)/clu.c

$(S)/cs.o: $(S)/cs.c

$(S)/refal.o: $(S)/refal.c

### End of Makefile
