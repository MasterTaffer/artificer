cmake_minimum_required (VERSION 2.6)
project (artificer)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --std=c++11 -Wall")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LinkerFlags}")

find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include(FindOpenGL REQUIRED)
include(FindLua REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

add_definitions(-DARTIFICE_STATIC_BUILD)

add_library(artificerLib SHARED artconf.cpp cartifice.cpp gl.cpp)
target_link_libraries(artificerLib ${LUA_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_STATIC_LIBRARIES})

add_executable(artificer main.cpp framesource.cpp)
target_link_libraries(artificer artificerLib)
