cmake_minimum_required(VERSION 3.0)
  
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

PROJECT(Geometric_Modeling)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories("/home/soumya/OpenMesh-2.4/install/include")

find_library(
    OpenMesh_LIBRARY
    NAMES OpenMesh
    HINTS "/home/soumya/OpenMesh-2.4/install/lib/OpenMesh/libOpenMeshCore.so")

file(GLOB CXX_FILES *.cpp *.cpp *.h)

add_executable(out ${CXX_FILES})

target_link_libraries(out ${OpenMesh_LIBRARY} -lGL -lGLU -lglut -lGLEW -lglui)