%VULKAN_SDK%\Bin\glslc shader.vert -o vert.spv
%VULKAN_SDK%\Bin\glslc shader.frag -o frag.spv

::I'm unsure if I need to compile two different sets of shaders for 32 and 64 bits
::eventually I'll want to copy the shaders over to all the different working directories
xcopy *.spv ..\WinOrb\shaders\ /y
xcopy *.spv ..\x64\Debug\shaders\ /y

pause