@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-31
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem set_env.bat
rem set environment variables for RefalAB
rem *******************************************************************

rem Check for spaces in the folder path
set "current_dir=%~dp0"
if not "%current_dir%"=="%current_dir: =%" (
    echo ============================================================
    echo [ERROR] The folder path contains spaces!
    echo Current path: %~dp0
    echo.
    echo Please rename the folders or move the project to a path
    echo without spaces ^(for example, C:\refalab\^).
    echo ============================================================
    pause
    exit /b
)

rem If there are no spaces, set environment variables
setx REFALABBIN %~dp0bin
setx REFALABLIB %~dp0lib
setx REFALABINCLUDE %~dp0include
setx REFALABCFLAGS "-pipe -Wall -O2"
setx REFALABCFLAGS_DBG "-pipe -Wall -O0 -g -Dmdebug"
setx REFALABLFLAGS "%~dp0lib\obj\mainrf.o -L%~dp0lib -lrefalab -pthread"
setx REFALABLFLAGS_DBG "%~dp0lib\obj\mainrf_dbg.o -L%~dp0lib -lrefalab_dbg -pthread"
setx REFALABLFLAGS_DEBUGGER "%~dp0lib\obj\rfdbg.o -L%~dp0lib -lrefalab -pthread"
setx REFALABLFLAGS_DEBUGGER_DBG "%~dp0lib\obj\rfdbg_dbg.o -L%~dp0lib -lrefalab_dbg -pthread"

echo ============================================================
echo [SUCCESS] All environment variables for RefalAB are set!
echo Please restart your command prompt (cmd) for changes to take effect.
echo ============================================================
pause
