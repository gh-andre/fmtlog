@echo off

setlocal

set FMT_VER=12.0.0
set BLD_DIR=%~1

if "%BLD_DIR%" == "" set BLD_DIR=.build

curl --location --output fmt-%FMT_VER%.tar.gz https://github.com/fmtlib/fmt/archive/refs/tags/%FMT_VER%.tar.gz
                                            
tar -xzf fmt-%FMT_VER%.tar.gz

del /Q fmt-%FMT_VER%.tar.gz

rem FMT_UNICODE only affects fmt libraries - see CMakeLists.txt for details
cmake -S fmt-%FMT_VER% -B %BLD_DIR%/fmt-build -DFMT_DOC=OFF -DFMT_TEST=OFF -DFMT_UNICODE=OFF

cmake --build %BLD_DIR%/fmt-build --config Debug
cmake --build %BLD_DIR%/fmt-build --config Release

cmake --install %BLD_DIR%/fmt-build --config Debug --prefix %BLD_DIR%/fmt
cmake --install %BLD_DIR%/fmt-build --config Release --prefix %BLD_DIR%/fmt

rmdir /S /Q fmt-%FMT_VER%
rmdir /S /Q %BLD_DIR%\fmt-build
