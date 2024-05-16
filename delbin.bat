@echo off
rem *******************************************************************
rem delbin.bat
rem 	delete temporary working files before refal2-distributive save
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 28.05.2005
rem *******************************************************************

cd  src\comp\
del *.asm 
del *.o 
del *.log

cd ..\inter\
del *.asm 
del *.o 
del *.log
del *.lst

cd ..\..\tests
del *.exe
del *.log
del *.lst
del *.asm
del *.o

