@echo off

rem Copyright 2024 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2024-10-22
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem set_env.bat
rem	Set Environments Variables for RefalAB
rem *******************************************************************

setx REFALABBIN %~dp0bin
setx REFALABLIB %~dp0lib
setx REFALABINCLUDE %~dp0include
setx REFALABCFLAGS "-pipe -Wall -O2"