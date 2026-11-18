@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-11-17
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabdclg.bat
rem Compile a RefalAB program, link it with the debugger initiator and run it under the debugger.
rem arg: %1 - name of the RefalAB program (without the extension .ref)
rem result: the program %1.exe is built and run (the log is in %1.log)
rem *******************************************************************

rem Compile to the object file through LLVM IR; the compiler listing goes to the log file.
bin\refalabc %1 -l,"-o %1.o -O2 -Wno-override-module" > %1.log
rem Link the object file with the debugger initiator and the library (statically, with pthread); the linker messages are appended to the log file.
clang -o %1.exe %1.o lib\obj\refalab_debugger.o -Llib -lrefalab -pthread -static >> %1.log
rem Run the program.
%1.exe
