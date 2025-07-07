@echo off

cd ../resources/shaders/
if not exist /bin (
    mkdir bin
)
glslc default.vert -o bin/default_vert.spv
glslc default.frag -o bin/default_frag.spv

echo "Compiled shaders!"