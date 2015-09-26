@echo off
CLS
ECHO.
ECHO ===============================================
ECHO Simple XYZ Thumbnail Shell Extension Installer
ECHO ===============================================

:: based on
:: https://stackoverflow.com/questions/12322308/
:: https://stackoverflow.com/questions/7044985/

setlocal DisableDelayedExpansion
set "batchPath=%~dp0"
setlocal EnableDelayedExpansion

:: 64bit check
IF "%PROCESSOR_ARCHITECTURE%"=="x86" (set bit=x86) else (set bit=amd64)

:: Elevate to admin and run regsvr32

ECHO Set UAC = CreateObject^("Shell.Application"^) > "%temp%\xyzgetPrivileges.vbs"
ECHO UAC.ShellExecute "regsvr32", """!batchPath!\Release\%bit%\EasyRpgXyzShellExtThumbnailHandler.dll""", "", "runas", 1 >> "%temp%\xyzgetPrivileges.vbs"
"%temp%\xyzgetPrivileges.vbs"
exit /B
