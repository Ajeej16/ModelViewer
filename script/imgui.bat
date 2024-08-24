@echo off

set lib_dir=..\lib

set imgui_dir=%lib_dir%\imgui\backends
set imguizmo_dir=%lib_dir%\ImGuizmo

set compiler_flags=/nologo /Zi /FC /EHsc

if not exist ..\build mkdir ..\build
pushd ..\build

cl /c %compiler_flags% /I%lib_dir%\imgui %lib_dir%\imgui\imgui.cpp /Fo:imgui.obj
cl /c %compiler_flags% /I%lib_dir%\imgui %lib_dir%\imgui\imgui_draw.cpp /Fo:imgui_draw.obj
cl /c %compiler_flags% /I%lib_dir%\imgui %lib_dir%\imgui\imgui_tables.cpp /Fo:imgui_tables.obj
cl /c %compiler_flags% /I%lib_dir%\imgui %lib_dir%\imgui\imgui_widgets.cpp /Fo:imgui_widgets.obj

cl /c %compiler_flags% /I%lib_dir%\imgui /I%lib_dir%\glfw\include\ %imgui_dir%\imgui_impl_glfw.cpp /Fo:imgui_glfw.obj
cl /c %compiler_flags% /I%lib_dir%\imgui %imgui_dir%\imgui_impl_opengl3.cpp /Fo:imgui_opengl3.obj
cl /c %compiler_flags% /I%lib_dir%\imgui /I%imguizmo_dir% %imguizmo_dir%\ImGuizmo.cpp /Fo:imguizmo.obj

lib /OUT:gui.lib imgui.obj imgui_draw.obj imgui_tables.obj imgui_widgets.obj imgui_glfw.obj imgui_opengl3.obj imguizmo.obj

popd