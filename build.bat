@echo off
setlocal
cd /d "%~dp0"
call "G:\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
msbuild.exe "qq-feiche-item-viewer.sln" /m /t:Build /p:Configuration=Release /p:Platform=x64
endlocal
