#!/usr/bin/env bash

cd $(dirname $0)/..
ROOT_DIR=$(pwd)
cd - > /dev/null

cd ${ROOT_DIR}/vendor/github.com/microsoft/vcpkg.git
if [ ! -f "vcpkg" ]; then
    ./bootstrap-vcpkg.sh
fi
export VCPKG_ROOT=$(pwd)
export PATH=${VCPKG_ROOT}:${PATH}
cd - > /dev/null

vcpkg add port sdl2
vcpkg add port sdl2-ttf

cd ${ROOT_DIR}
rm -rf build
cmake --preset=default
cmake --build build
