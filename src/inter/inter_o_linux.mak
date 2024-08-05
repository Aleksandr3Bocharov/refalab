#############################################################################
# inter_o_linux.mak
#	Makefile for mainrf.o rfdbg.o creation,
#	Version for UNIX ( FreeBSD, Linux )
#
# Copyright(C) Aleksandr Bocharov
# Author : Aleksandr Bocharov
# Date : 05.08.2024
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c

CC	=	gcc
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
