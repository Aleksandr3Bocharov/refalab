@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-14
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem refplatform.bat
rem special script
rem compile xplatformwin.ref from refal to object file
rem result: xplatform.o  
rem *******************************************************************

bin\RefalABc src\inter\xplatformwin -a,"-o src\inter\xplatform.o" > src\inter\xplatform.log
