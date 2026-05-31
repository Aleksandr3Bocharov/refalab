@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-30
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem set_env.bat
rem set environments variables for RefalAB
rem *******************************************************************

setx REFALABBIN %~dp0bin
setx REFALABLIB %~dp0lib
setx REFALABINCLUDE %~dp0include
setx REFALABCFLAGS "-pipe -Wall -O2"
setx REFALABCFLAGS_DBG "-pipe -Wall -O0 -g -Dmdebug"