@echo off
gcc -Wall -o test source\main.c
if not %ERRORLEVEL% == 0 goto build_err
test
goto end

:build_err
echo "compile failed."

:end

