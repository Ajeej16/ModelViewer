@echo off

set application_name=OpenGLViewer
set build_options=
set lib_dir=..\lib

set includes=/I%lib_dir% /I%lib_dir%\glew\include /I%lib_dir%\glfw\include /I%lib_dir%\imgui /I%lib_dir%\imgui\backends /I%lib_dir%\ImGuizmo
set libs=/LIBPATH:%lib_dir%\glew\lib\Release\x64\ /LIBPATH:%lib_dir%\glfw\lib-static-ucrt\

set compiler_flags=/nologo /Zi /FAsc /EHsc
set linker_flags=/opt:ref /incremental:no glfw3dll.lib glew32.lib opengl32.lib user32.lib gdi32.lib shell32.lib kernel32.lib gui.lib

set glfw_dll=%lib_dir%\glfw\lib-static-ucrt
set glew_dll=%lib_dir%\glew\bin\Release\x64

if not exist ..\build mkdir ..\build
pushd ..\build

copy %glfw_dll%\glfw3.dll .\
copy %glew_dll%\glew32.dll .\

call ..\script\imgui.bat

cl %compiler_flags% %includes% ..\src\main.cpp /link %libs% %linker_flags% /out:%application_name%.exe

popd