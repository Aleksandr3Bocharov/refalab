@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxsys2.bat
rem special script
rem compile xsys2.ref from refal to object file
rem result: xsys2.o  
rem *******************************************************************

bin\refalabc src\inter\xsys2 -a,"-o src\inter\xsys2.o" > src\inter\xsys2.log
