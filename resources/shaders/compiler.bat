REM vertex: glslangValidator -V VertexShader.vert -o vert.spv
REM compute: glslangValidator -V fogVolumetric.comp -o fogVolumetricCompute.spv
REM task: glslangValidator -V --target-env vulkan1.3 -S task particleTask.task -o particleTask.spv
REM mesh: glslangValidator -V --target-env vulkan1.3 -S mesh particleMeshShader.mesh -o particleMesh.spv
REM fragment: glslangValidator -V FragmentShader.frag -o frag.spv

glslangValidator -V PostProcess.vert -o vert.spv

glslangValidator -V RayCast.comp -o compute.spv

glslangValidator -V FinalImageFrag.frag -o frag.spv
pause