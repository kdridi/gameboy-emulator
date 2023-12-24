#!/usr/bin/env bash

cd $(dirname $0)/..
ROOT_DIR=$(pwd)
cd - > /dev/null

cd ${ROOT_DIR}/vendor/github.com/microsoft/vcpkg.git
export VCPKG_ROOT=$(pwd)
export PATH=${VCPKG_ROOT}:${PATH}
cd - > /dev/null

# bootstrap-vcpkg.sh
# vcpkg new --application
# vcpkg add port fmt
# vcpkg add port sfml

cd ${ROOT_DIR}
rm -rf build
cmake --preset=default
cmake --build build