@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-11-05
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxbool.bat
rem special script
rem compile xbool.ref from refal to object file
rem result: xbool.o  
rem *******************************************************************

bin\refalabc src\inter\xbool -a,"-o src\inter\xbool.o" > src\inter\xbool.log
