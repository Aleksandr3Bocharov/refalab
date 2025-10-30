@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-30
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxmo2.bat
rem special script
rem compile xmo2.ref from refal to object file
rem result: xmo2.o  
rem *******************************************************************

bin\refalabc src\inter\xmo2 -a,"-o src\inter\xmo2.o" > src\inter\xmo2.log
