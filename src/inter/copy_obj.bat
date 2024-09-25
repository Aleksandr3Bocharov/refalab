@echo off

rem Copyright 2024 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2024-09-25
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem copy_obj.bat
rem 	copy mainrf.o and rfdbg.o
rem *******************************************************************

copy /y src\inter\mainrf.o lib\
copy /y src\inter\rfdbg.o lib\
