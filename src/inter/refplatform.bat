@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-03-10
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem refplatform.bat
rem special script
rem compile xplatformwin.ref from refal to GNU-assembler + assembling
rem result: xplatform.o  
rem *******************************************************************

bin\RefalAB src\inter\xplatformwin > src\inter\xplatform.log
as -o src\inter\xplatform.o src\inter\xplatformwin.s >> src\inter\xplatform.log 
