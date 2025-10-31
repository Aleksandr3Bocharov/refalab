@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-31
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxcomp2.bat
rem special script
rem compile xcomp2.ref from refal to object file
rem result: xcomp2.o  
rem *******************************************************************

bin\refalabc src\inter\xcomp2 -a,"-o src\inter\xcomp2.o" > src\inter\xcomp2.log