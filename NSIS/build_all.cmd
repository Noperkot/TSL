@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
pushd %~dp0

FOR /F "TOKENS=*" %%F IN ('DIR /O:N /B /AD "TorrServers\*"') DO (
    SET dirName=%%~F
    echo Building TorrServer !dirName!
    makensis.exe /V3 /DTS_VERSION=!dirName! offline_Installer.nsi
    echo.
    echo.
)

popd
ENDLOCAL
pause


