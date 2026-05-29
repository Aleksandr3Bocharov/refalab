@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxar2.bat
rem special script
rem compile xar2.ref from refal to object file
rem result: xar2.o  
rem *******************************************************************

bin\refalabc src\inter\xar2 -a,"-o src\inter\xar2.o" > src\inter\xar2.log