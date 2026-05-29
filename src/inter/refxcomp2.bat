@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refxcomp2.bat
rem special script
rem compile xcomp2.ref from refal to object file
rem result: xcomp2.o  
rem *******************************************************************

bin\refalabc src\inter\xcomp2 -a,"-o src\inter\xcomp2.o" > src\inter\xcomp2.log