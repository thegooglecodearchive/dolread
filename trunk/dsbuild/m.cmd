@echo off

build.py

if not %ERRORLEVEL% == 0 goto build_err
dsemu DolphinReader_fs.ds.gba
goto end

:build_err
echo ===buiding error===
pause

:end
