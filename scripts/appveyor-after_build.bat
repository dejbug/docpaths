@echo off

echo -- running "after_build.bat"

echo -- creating build folder
mkdir build

echo -- moving binaries into build folder
move docpaths.exe build\

echo -- build folder ready for deployment
