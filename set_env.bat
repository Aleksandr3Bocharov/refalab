@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-06-11
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
setx REFALABLLFLAGS "-O2 -Wno-override-module"
setx REFALABLLFLAGS_DBG "-O0 -g -Wno-override-module"
setx REFALABCFLAGS "-pipe -Wall -O2"
setx REFALABCFLAGS_DBG "-pipe -Wall -O0 -g -Dmdebug"
setx REFALABLFLAGS "%~dp0lib\obj\refalab_initiator.o -L%~dp0lib -lrefalab -pthread"
setx REFALABLFLAGS_DBG "%~dp0lib\obj\refalab_initiator_dbg.o -L%~dp0lib -lrefalab_dbg -pthread"
setx REFALABLFLAGS_DEBUGGER "%~dp0lib\obj\refalab_debugger.o -L%~dp0lib -lrefalab -pthread"
setx REFALABLFLAGS_DEBUGGER_DBG "%~dp0lib\obj\refalab_debugger_dbg.o -L%~dp0lib -lrefalab_dbg -pthread"

rem Prompt block to ask for adding to PATH
echo.
choice /M "Do you want to add the bin folder to your user PATH?"
if %errorlevel% equ 1 (
rem Turn on delayed expansion to safely handle paths with parentheses (like x86)
    setlocal enabledelayedexpansion
    
    echo !PATH! | findstr /I /C:"%~dp0bin" >nul
    if !errorlevel! neq 0 (
rem Extract the exact current user PATH safely
        for /f "tokens=2*" %%A in ('reg query HKCU\Environment /v PATH 2^>nul') do set "USERPATH=%%B"
        
        if defined USERPATH (
            setx PATH "!USERPATH!;%~dp0bin"
        ) else (
            setx PATH %~dp0bin
        )
        echo [SUCCESS] Path to bin folder added to PATH.
    ) else (
        echo [INFO] Path to bin folder is already in PATH.
    )
    
    endlocal
) else (
    echo [INFO] Skipping PATH modification.
)

echo ============================================================
echo [SUCCESS] All environment variables for RefalAB are set!
echo [IMPORTANT] Please restart your command prompt (cmd) for changes to take effect.
echo ============================================================
pause
exit /b 0