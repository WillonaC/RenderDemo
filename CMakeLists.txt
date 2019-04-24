cmake_minimum_required(VERSION 3.9)

project(RenderDemo)

add_subdirectory(thirdparty)

# main target.
add_executable(Rendering
	src/main.cpp
    src/render_hair.cpp
    src/render_hair.h
	src/render_mesh.cpp
	src/render_mesh.h
    src/Shader.cc
    src/Shader.h
    src/tiny_obj_loader.cc
    src/tiny_obj_loader.h
	src/tool.cc
	src/tool.h
    src/xy_calc.h
    src/xy_ext.h
    src/xy_glfw.cc
    src/xy_glfw.h
    src/xy_opengl.cc
    src/xy_opengl.h
    src/xy_rt.h)

target_include_directories(Rendering
PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(Rendering glad)
target_link_libraries(Rendering glfw)
target_link_libraries(Rendering stb)
target_link_libraries(Rendering imgui)

install(TARGETS Rendering
    DESTINATION bin)
