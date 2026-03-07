@echo off
:clean
echo Cleaning
rmdir build /s /q
rmdir obj /s /q
rmdir libs /s /q
set cleaned=cleaned
if exist %cd%/build (
echo Failed to clean
goto clean
) else (
echo Cleaned
)

if not defined cleaned (
goto clean
)