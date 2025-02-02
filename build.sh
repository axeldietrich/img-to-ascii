#!/bin/bash
if [ -d "/project-build" ]; then
    cd project-build
else
    mkdir project-build
    cd project-build
fi
cmake -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ ..
make
