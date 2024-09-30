@echo off

rem Copyright 2024 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2024-09-30
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem refext.bat
rem 	special script
rem	compile ext.ref from refal to GNU-assembler + assembling. 
rem	result: ext.o  
rem *******************************************************************

bin\RefalAB src\inter\ext > src\inter\ext.log
as -o src\inter\ext.o src\inter\ext.asm >> src\inter\ext.log 
