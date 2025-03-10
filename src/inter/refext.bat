@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-03-10
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem refext.bat
rem special script
rem compile xext.ref from refal to GNU-assembler + assembling
rem result: xext.o  
rem *******************************************************************

bin\RefalAB src\inter\xext > src\inter\xext.log
as -o src\inter\xext.o src\inter\xext.s >> src\inter\xext.log 
