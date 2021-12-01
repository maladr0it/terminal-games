#!/usr/bin/env bash

rm -rf build
mkdir build

clang -g -Wall -O3 $1/**.c lib/**.c -o build/main