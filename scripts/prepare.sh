#!/usr/bin/env bash

set -e
set -x

cd $(dirname $0)/..
ROOT_DIR=$(pwd)
cd - > /dev/null

cd ${ROOT_DIR}/vendor/github.com/microsoft/vcpkg.git
if [ ! -f "vcpkg" ]; then
    ./bootstrap-vcpkg.sh
fi
export VCPKG_ROOT=$(pwd)
export PATH=${VCPKG_ROOT}:${PATH}
export LLVM_ROOT=/opt/homebrew/Cellar/llvm@16/16.0.6
cd - > /dev/null

vcpkg add port gtest
vcpkg add port sdl2
vcpkg add port sdl2-ttf

cd ${ROOT_DIR}
rm -rf build

cmake --preset=llvm && 
    ${LLVM_ROOT}/bin/scan-build cmake --build build/llvm

cmake --preset=makefile && 
    ${LLVM_ROOT}/bin/scan-build cmake --build build/makefile

cmake --preset=xcode && 
    ${LLVM_ROOT}/bin/scan-build cmake --build build/xcode

