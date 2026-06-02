@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-31
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem set_env.bat
rem set environments variables for RefalAB
rem *******************************************************************

@echo off

:: Check for spaces in the folder path
echo %~dp0 | findstr /c:" " >nul
if %errorlevel%==0 (
    echo ============================================================
    echo [ERROR] The folder path contains spaces!
    echo Current path: %~dp0
    echo.
    echo Please rename the folders or move the project to a path
    echo without spaces (for example, C:\refalab\).
    echo ============================================================
    pause
    exit /b
)

:: If there are no spaces, set environment variables
setx REFALABBIN %~dp0bin
setx REFALABLIB %~dp0lib
setx REFALABINCLUDE %~dp0include
setx REFALABCFLAGS "-pipe -Wall -O2"
setx REFALABCFLAGS_DBG "-pipe -Wall -O0 -g -Dmdebug"
setx REFALABLFLAGS "%~dp0lib\obj\mainrf.o -L%~dp0lib -lrefalab -pthread"

echo ============================================================
echo [SUCCESS] All environment variables for RefalAB are set!
echo Please restart your command prompt (cmd) for changes to take effect.
echo ============================================================
pause
