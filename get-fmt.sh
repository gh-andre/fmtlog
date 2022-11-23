#!/bin/bash

FMT_VER=9.1.0
BLD_DIR=$1

if [ "$BLD_DIR" == "" ]
then
  BLD_DIR=build
fi

curl --location --output fmt-$FMT_VER.tar.gz https://github.com/fmtlib/fmt/archive/refs/tags/$FMT_VER.tar.gz
                                            
tar -xzf fmt-$FMT_VER.tar.gz

rm -f fmt-$FMT_VER.tar.gz

#
# Debug
#
cmake -S fmt-$FMT_VER -B $BLD_DIR/fmt-build -DCMAKE_BUILD_TYPE=Debug -DFMT_DOC=OFF -DFMT_TEST=OFF

cmake --build $BLD_DIR/fmt-build

# remove --prefix to install to /usr/local
cmake --install $BLD_DIR/fmt-build --prefix $BLD_DIR/fmt

cp $BLD_DIR/fmt-build/install_manifest.txt $BLD_DIR/install_manifest_debug.txt

#
# Release
#
cmake -S fmt-$FMT_VER -B $BLD_DIR/fmt-build -DCMAKE_BUILD_TYPE=Release -DFMT_DOC=OFF -DFMT_TEST=OFF

cmake --build $BLD_DIR/fmt-build

# remove --prefix to install to /usr/local
cmake --install $BLD_DIR/fmt-build --prefix $BLD_DIR/fmt

cp $BLD_DIR/fmt-build/install_manifest.txt $BLD_DIR/install_manifest_release.txt

rm -rf fmt-$FMT_VER
rm -rf $BLD_DIR/fmt-build
