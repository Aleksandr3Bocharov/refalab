@echo off
rem *******************************************************************
rem delbin.bat
rem 	delete temporary working files
rem
rem Copyright (C) Aleksandr Bocharov
rem Author: Aleksandr Bocharov
rem Date: 12.07.2024
rem *******************************************************************

cd  src\inter\
del *.asm 
del *.o 
del *.log
del *.lst


