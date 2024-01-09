@echo off
for /r %%f in (*.vert) do C:/VulkanSDK/1.3.236.0/Bin/glslc.exe %%f -o %%~nf" Vert.spv"
for /r %%f in (*.frag) do C:/VulkanSDK/1.3.236.0/Bin/glslc.exe %%f -o %%~nf" Frag.spv"
for /r %%f in (*.comp) do C:/VulkanSDK/1.3.236.0/Bin/glslc.exe %%f -o %%~nf" Comp.spv"
for %%f in (*.spv) do MOVE /Y "%%f" ..\..\Binassets\APBuildingBlocks >nul