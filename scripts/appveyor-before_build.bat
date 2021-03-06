@echo off

echo -- running "before_build.bat"

echo -- setting MINGW paths
set MINGW_BIN=C:\mingw-w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\mingw32\bin
set PATH=%MINGW_BIN%;%PATH%

echo -- renaming mingw32-make.exe in-place
PUSHD %MINGW_BIN%
rename mingw32-make.exe make.exe 
POPD

echo -- fetching submodules
git submodule update --init --recursive

echo -- project is ready for make
