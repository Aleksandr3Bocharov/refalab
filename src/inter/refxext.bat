@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxext.bat
rem special script
rem compile xext.ref from refal to object file
rem result: xext.o  
rem *******************************************************************

bin\refalabc src\inter\xext -a,"-o src\inter\xext.o" > src\inter\xext.log
