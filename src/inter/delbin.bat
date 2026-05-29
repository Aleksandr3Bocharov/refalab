@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem delbin.bat
rem delete temporary working files
rem *******************************************************************

cd  src\inter\
del *.s
del *.o
del *.log
del *.lst
