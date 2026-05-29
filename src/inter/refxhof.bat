@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxhof.bat
rem special script
rem compile xhof.ref from refal to object file
rem result: xhof.o  
rem *******************************************************************

bin\refalabc src\inter\xhof -a,"-o src\inter\xhof.o" > src\inter\xhof.log