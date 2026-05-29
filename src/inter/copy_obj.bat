@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-29
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem copy_obj.bat
rem copy mainrf.o and rfdbg.o
rem *******************************************************************

copy /y src\inter\mainrf.o lib\obj
copy /y src\inter\rfdbg.o lib\obj
