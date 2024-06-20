#############################################################################
#inter.mak
#Makefile for interpreter library(librefal2.a) creation,
#result lib\librefal2.a library
#Version for Windows(Win95 / 98 / ME / NT / 2000 / XP)
#
#Copyright(C) Refal - 2 Team
#Author : Leonid Belous(aka BLF)
#E - mail : belous @ilt.kharkov.ua
#Date : 19.06.2024
#############################################################################

####### Compiler, tools and options

.SUFFIXES: .c .ref

CC	=	gcc
CFLAGS	=	-pipe -Wall -O2 -DNO_DEBUG
AR	=	ar
REFXCV	=	src\inter\refxcv.bat 	# refal2 + assembler for xcv
TAR	=	tar -cf
GZIP	=	gzip -9f
DEL	=	del
REFLIB	=	lib\librefal2.a
USERLIB	=	lib\libr2user.a
S	=      	src\inter
OBJ	=	lib\r2objects
####### Files

SOURCES =		\
	$(S)\d.c	\
	$(S)\mainrf.c 	\
	$(S)\rfdbg.c 	\
	$(S)\rfintf.c 	\
	$(S)\rfrun1.c 	\
	$(S)\rfrun2.c 	\
	$(S)\xapply.c 	\
	$(S)\xar.c 	\
	$(S)\xcf.c 	\
	$(S)\xcoden.c 	\
	$(S)\xgcd.c 	\
	$(S)\xjak.c 	\
	$(S)\xmo.c 	\
	$(S)\xrename.c 	\
	$(S)\xtime.c 	\
	$(S)\xvv.c 	\
	$(S)\xvv5.c 	\
	$(S)\xcv.ref

OBJECTS =		\
	$(S)\d.o 	\
	$(S)\mainrf.o 	\
	$(S)\rfdbg.o 	\
	$(S)\rfintf.o 	\
	$(S)\rfrun1.o 	\
	$(S)\rfrun2.o 	\
	$(S)\xapply.o 	\
	$(S)\xar.o 	\
	$(S)\xcf.o 	\
	$(S)\xcoden.o 	\
	$(S)\xcv.o 	\
	$(S)\xgcd.o 	\
	$(S)\xjak.o 	\
	$(S)\xmo.o 	\
	$(S)\xrename.o 	\
	$(S)\xtime.o 	\
	$(S)\xvv.o 	\
	$(S)\xvv5.o

####### Implicit rules

.ref.o: 
	$(REFXCV)  
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

####### Build rules

all: $(REFLIB)

$(REFLIB): $(OBJECTS)
	$(AR) rs $(REFLIB) $(OBJECTS) 
	$(AR) rs $(USERLIB)
	-copy $(S)\mainrf.o $(OBJ)
	-copy $(S)\rfdbg.o $(OBJ)


####### Dependences

$(S)\d.o:	$(S)\d.c $(S)\refal.def $(S)\d.h

$(S)\mainrf.o: 	$(S)\mainrf.c

$(S)\rfdbg.o:	$(S)\rfdbg.c $(S)\refal.def

$(S)\rfintf.o: 	$(S)\rfintf.c $(S)\refal.def

$(S)\rfrun1.o: 	$(S)\rfrun1.c $(S)\refal.def

$(S)\rfrun2.o: 	$(S)\rfrun2.c $(S)\refal.def

$(S)\xapply.o: 	$(S)\xapply.c $(S)\refal.def

$(S)\xar.o: 	$(S)\xar.c $(S)\refal.def

$(S)\xcf.o: 	$(S)\xcf.c $(S)\refal.def

$(S)\xcoden.o: 	$(S)\xcoden.c $(S)\refal.def

$(S)\xgcd.o: 	$(S)\xgcd.c $(S)\refal.def

$(S)\xjak.o: 	$(S)\xjak.c $(S)\refal.def

$(S)\xmo.o: 	$(S)\xmo.c $(S)\refal.def

$(S)\xrename.o: $(S)\xrename.c $(S)\refal.def

$(S)\xtime.o: 	$(S)\xtime.c $(S)\refal.def

$(S)\xvv.o: 	$(S)\xvv.c $(S)\refal.def

$(S)\xvv5.o: 	$(S)\xvv5.c $(S)\refal.def


####### End of Makefile
