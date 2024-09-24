@echo off
rem *******************************************************************
rem RefalABc.bat
rem	compile from refal to GNU-assembler 
rem 	arg: %1 - name of refal programm (without extension ref)
rem	result: %1.asm in GNU assembler language 
rem
rem Copyright(C) Aleksandr Bocharov
rem Author : Aleksandr Bocharov
rem Date: 24.09.2024
rem *******************************************************************

bin\RefalAB %1 > %1.log
