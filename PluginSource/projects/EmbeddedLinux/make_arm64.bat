REM UNITY_ROOT should be set to folder with Unity repository
"%UNITY_ROOT%/build/EmbeddedLinux/llvm/bin/clang++" --sysroot="%UNITY_ROOT%/build/EmbeddedLinux/sdk-linux-arm64/aarch64-embedded-linux-gnu/sysroot" -DUNITY_EMBEDDED_LINUX=1 -DSUPPORT_VULKAN=1 -I"%UNITY_ROOT%/External/Vulkan/include" -O2 -fPIC -shared -rdynamic -o libRenderingPlugin.so -fuse-ld=lld.exe -Wl,-soname,RenderingPlugin -Wl,-lGLESv2 --gcc-toolchain="%UNITY_ROOT%/build/EmbeddedLinux/sdk-linux-arm64" -target aarch64-embedded-linux-gnu ../../source/RenderingPlugin.cpp ../../source/RenderAPI_OpenGLCoreES.cpp ../../source/RenderAPI_Vulkan.cpp ../../source/RenderAPI.cpp
