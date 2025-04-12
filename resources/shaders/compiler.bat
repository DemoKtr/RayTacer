REM vertex: glslangValidator -V VertexShader.vert -o vert.spv
REM compute: glslangValidator -V fogVolumetric.comp -o fogVolumetricCompute.spv
REM task: glslangValidator -V --target-env vulkan1.3 -S task particleTask.task -o particleTask.spv
REM mesh: glslangValidator -V --target-env vulkan1.3 -S mesh particleMeshShader.mesh -o particleMesh.spv
REM fragment: glslangValidator -V FragmentShader.frag -o frag.spv

glslangValidator -V PostProcess.vert -o vert.spv

glslangValidator -V RayCast.comp -o compute.spv

glslangValidator -V FinalImageFrag.frag -o frag.spv

glslangValidator -V --target-env vulkan1.3 raygen.rgen -o raygen.spv
glslangValidator -V --target-env vulkan1.3 miss.rmiss -o miss.spv
glslangValidator -V --target-env vulkan1.3 closesthit.rchit -o closesthit.spv
glslangValidator -V --target-env vulkan1.3 -S rahit anyhit_shadow.rahit -o anyhit_shadow.spv
glslangValidator -V --target-env vulkan1.3 miss_shadow.rmiss -o miss_shadow.spv


pause