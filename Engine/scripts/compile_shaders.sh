#!/bin/zsh
cd ../resources/shaders/
if [[ ! -d "bin" ]]
then
    mkdir bin
fi
glslc default.vert -o bin/default_vert.spv
glslc default.frag -o bin/default_frag.spv