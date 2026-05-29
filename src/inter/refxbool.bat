@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxbool.bat
rem special script
rem compile xbool.ref from refal to object file
rem result: xbool.o  
rem *******************************************************************

bin\refalabc src\inter\xbool -a,"-o src\inter\xbool.o" > src\inter\xbool.log
