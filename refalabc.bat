@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-14
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabc.bat
rem compile from refal to object file 
rem arg: %1 - name of refal programm (without extension ref)
rem result: %1.o
rem *******************************************************************

bin\refalabc %1 -a,"-o %1.o" > %1.log
