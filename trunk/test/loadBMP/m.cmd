@echo off
make
if not %ERRORLEVEL% == 0 goto build_err

:pafs
pafs loadBMP.ds.gba bitmap 2048
if not %ERRORLEVEL% == 0 goto pafs_err

dsemu NewloadBMP.ds.gba
goto end

:pafs_err
echo ===pafs error===
goto end

:build_err
echo ===buiding error===

:end
