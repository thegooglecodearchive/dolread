@echo off
make
if not %ERRORLEVEL% == 0 goto build_err

:pafs
pafs loadBMP.nds bitmap
if not %ERRORLEVEL% == 0 goto pafs_err

cp NewloadBMP.nds m:
goto end

:pafs_err
echo ===pafs error===
goto end

:build_err
echo ===buiding error===

:end
