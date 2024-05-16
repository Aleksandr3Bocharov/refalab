@echo off
rem *******************************************************************
rem refxcv.bat
rem 	special part for comp.mak
rem	compile xcv.ref from refal to GNU-assembler + assembling. 
rem	result: xcv.o  
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 28.05.2005
rem *******************************************************************

bin\refal2 src\inter\xcv.ref > src\inter\xcv.log
as -o src\inter\xcv.o src\inter\xcv.asm >> src\inter\xcv.log 
