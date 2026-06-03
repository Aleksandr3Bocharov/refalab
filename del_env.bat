@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-05-31
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem del_env.bat
rem delete environment variables for RefalAB
rem *******************************************************************

echo Deleting RefalAB environment variables...

rem 1. Permanently delete user environment variables from the Windows Registry
reg delete "HKCU\Environment" /f /v REFALABBIN >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABLIB >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABINCLUDE >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABCFLAGS >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABCFLAGS_DBG >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABLFLAGS >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABLFLAGS_DBG >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABLFLAGS_DEBUGGER >nul 2>&1
reg delete "HKCU\Environment" /f /v REFALABLFLAGS_DEBUGGER_DBG >nul 2>&1

echo [SUCCESS] Basic variables removed from Registry.

rem 2. Safe removal of the bin folder from the User PATH using PowerShell
echo Cleaning up User PATH...
set "target_bin=%~dp0bin"

powershell -NoProfile -Command ^
    "$path = [Environment]::GetEnvironmentVariable('PATH', 'User'); " ^
    "if ($path) { " ^
    "  $target = '%target_bin%'.TrimEnd('\'); " ^
    "  $elements = $path -split ';' | Where-Object { $_.TrimEnd('\') -ne $target }; " ^
    "  $newPath = $elements -join ';'; " ^
    "  [Environment]::SetEnvironmentVariable('PATH', $newPath, 'User'); " ^
    "}"

echo [SUCCESS] Folder removed from PATH.

rem 3. Clear variables in the current session (for immediate effect)
set REFALABBIN=
set REFALABLIB=
set REFALABINCLUDE=
set REFALABCFLAGS=
set REFALABCFLAGS_DBG=
set REFALABLFLAGS=
set REFALABLFLAGS_DBG=
set REFALABLFLAGS_DEBUGGER=
set REFALABLFLAGS_DEBUGGER_DBG=

echo [INFO] Done.
echo [IMPORTANT] Please restart your command prompt to apply changes fully.
pause
exit /b 0