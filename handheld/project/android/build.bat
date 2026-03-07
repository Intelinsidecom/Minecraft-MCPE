@echo off


if exist %cd%/build (
echo Files Already exist, do you want to clean first to make sure the newest files are placed?
choice
if not %errorlevel% == 1 (
  clean.bat
  goto build
) else (
  goto build
)
)

:build
ndk-build
gradlew build