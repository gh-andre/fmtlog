#!/bin/bash

BUILD_DIR=${BUILD_DIR:-.build}

# Git submodule fmt must be initialized (see CMakeLists.txt for details)
mkdir -p "$BUILD_DIR" \
  && cd "$BUILD_DIR" \
  && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
       -DCMAKE_BUILD_TYPE=Release \
       -DFMTLOG_BUILD_TEST=ON \
       -DFMTLOG_BUNDLED_FMT=ON \
       ..\
  && cmake --build . -j \
  && cmake --install . --prefix .
