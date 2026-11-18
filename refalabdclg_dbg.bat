@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-11-17
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabdclg_dbg.bat
rem Compile a RefalAB program, link it with the debugger initiator and run it under the debugger.
rem Debug version: no optimization, debug information, the debug library and the debug trace.
rem arg: %1 - name of the RefalAB program (without the extension .ref)
rem result: the program %1_dbg.exe is built and run (the log is in %1_dbg.log)
rem *******************************************************************

rem Compile to the object file through LLVM IR; the compiler listing goes to the log file.
bin\refalabc %1 -l,"-o %1_dbg.o -O0 -g -Wno-override-module" > %1_dbg.log
rem Link the object file with the debugger initiator and the library (statically, with pthread); the linker messages are appended to the log file.
clang -o %1_dbg.exe %1_dbg.o lib\obj\refalab_debugger_dbg.o -Llib -lrefalab_dbg -pthread -static >> %1_dbg.log
rem Run the program.
%1_dbg.exe
