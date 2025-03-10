@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-03-10
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem RefalABc.bat
rem compile from refal to GNU-assembler 
rem arg: %1 - name of refal programm (without extension ref)
rem result: %1.s in GNU assembler language 
rem *******************************************************************

bin\RefalAB %1 > %1.log
