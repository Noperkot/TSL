@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
pushd %~dp0

FOR /F "TOKENS=*" %%F IN ('DIR /B /AD "TorrServers\*"') DO (
    SET dirName=%%~F
    echo Building TorrServer !dirName!
    makensis.exe /V3 /DAPPVERSION=!dirName! TorrServer.nsi
    echo.
    echo.
)

popd
ENDLOCAL
pause


