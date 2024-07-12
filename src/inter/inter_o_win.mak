#############################################################################
#inter_o_win.mak
#Makefile for mainrf.o rfdbg.o creation,
#Version for Windows(Win95 / 98 / ME / NT / 2000 / XP)
#
#Copyright(C) Aleksandr Bocharov
#Author : Aleksandr Bocharov
#Date : 11.07.2024
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c

CC	=	gcc
CFLAGS	=	-pipe -Wall -O2 
S	=      	src\inter
OBJ	=	lib\
####### Files

SOURCES =		\
	$(S)\mainrf.c 	\
	$(S)\rfdbg.c 	

OBJECTS =		\
	$(S)\mainrf.o 	\
	$(S)\rfdbg.o 

####### Implicit rules

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

####### Build rules

all: $(OBJECTS)

####### Dependences

$(S)\mainrf.o: 	$(S)\mainrf.c

$(S)\rfdbg.o:	$(S)\rfdbg.c $(S)\refal.def

####### End of Makefile
