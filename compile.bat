pushd bin\Debug\shaders
glslc.exe  triangle.vert -o vert.spv
glslc.exe  triangle.frag -o frag.spv
popd
pause