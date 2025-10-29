@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxhof.bat
rem special script
rem compile xhof.ref from refal to object file
rem result: xhof.o  
rem *******************************************************************

bin\refalabc src\inter\xhof -a,"-o src\inter\xhof.o" > src\inter\xhof.log