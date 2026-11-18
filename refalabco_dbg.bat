@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-11-17
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabco_dbg.bat
rem Compile a RefalAB program to an object file.
rem Debug version: no optimization, debug information, the debug library and the debug trace.
rem arg: %1 - name of the RefalAB program (without the extension .ref)
rem result: %1_dbg.o (the compiler listing is in %1_dbg.log)
rem *******************************************************************

rem Compile to the object file through LLVM IR; the compiler listing goes to the log file.
bin\refalabc %1 -l,"-o %1_dbg.o -O0 -g -Wno-override-module" > %1_dbg.log
