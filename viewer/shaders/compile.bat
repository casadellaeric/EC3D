@echo off
for /r %%i in (*.vert) do %VULKAN_SDK%\Bin\glslc.exe %%i -o %%~ni-vert.spv
for /r %%i in (*.frag) do %VULKAN_SDK%\Bin\glslc.exe %%i -o %%~ni-frag.spv
pause