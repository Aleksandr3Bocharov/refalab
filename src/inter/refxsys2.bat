@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-11-03
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxsys2.bat
rem special script
rem compile xsys2.ref from refal to object file
rem result: xsys2.o  
rem *******************************************************************

bin\refalabc src\inter\xsys2 -a,"-o src\inter\xsys2.o" > src\inter\xsys2.log
