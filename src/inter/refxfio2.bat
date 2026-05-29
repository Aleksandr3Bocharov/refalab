@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxfio2.bat
rem special script
rem compile xfio2.ref from refal to object file
rem result: xfio2.o  
rem *******************************************************************

bin\refalabc src\inter\xfio2 -a,"-o src\inter\xfio2.o" > src\inter\xfio2.log
