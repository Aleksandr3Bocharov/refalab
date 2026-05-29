@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxplatformwin.bat
rem special script
rem compile xplatformwin.ref from refal to object file
rem result: xplatform.o  
rem *******************************************************************

bin\refalabc src\inter\xplatformwin -a,"-o src\inter\xplatform.o" > src\inter\xplatform.log
