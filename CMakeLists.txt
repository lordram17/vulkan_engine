cmake_minimum_required(VERSION 3.16)
project(ivr)

set(CMAKE_CXX_STANDARD 17)

#including all files in the src directory. Note : Cmake discourages this apparently but I dont know a different way of doing this
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(ivr ${SRC_FILES})

#adding the include directory for the headers. PBULIC is the scope, not sure what it does
target_include_directories(ivr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ivr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external_libs)

target_link_libraries(ivr -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)


