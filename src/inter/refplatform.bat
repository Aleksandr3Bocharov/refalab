@echo off

rem Copyright 2024 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2024-09-30
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem refplatform.bat
rem 	special script
rem	compile platformwin.ref from refal to GNU-assembler + assembling. 
rem	result: platform.o  
rem *******************************************************************

bin\RefalAB src\inter\platformwin > src\inter\platform.log
as -o src\inter\platform.o src\inter\platformwin.asm >> src\inter\platform.log 
