# Copyright 2024 Aleksandr Bocharov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# 2024-10-31
# https://github.com/Aleksandr3Bocharov/RefalAB

#############################################################################
# inter_o_linux.mak
#	Makefile for mainrf.o rfdbg.o creation,
#	Version for Linux ( UNIX, FreeBSD )
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c

CC	=	clang
CFLAGS	=	-pipe -Wall -O2 -DUNIX
S	=      	src/inter
####### Files

SOURCES =		\
	$(S)/mainrf.c 	\
	$(S)/rfdbg.c 	

OBJECTS =		\
	$(S)/mainrf.o 	\
	$(S)/rfdbg.o 

####### Implicit rules

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

####### Build rules

all: $(OBJECTS)

####### Dependences

$(S)/mainrf.o: 	$(S)/mainrf.c

$(S)/rfdbg.o:	$(S)/rfdbg.c $(S)/refal.def

####### End of Makefile
