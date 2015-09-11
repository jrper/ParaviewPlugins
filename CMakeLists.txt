cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MACOSX_RPATH 1)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

file(MAKE_DIRECTORY plugins)

add_subdirectory(ShowCVs)
add_subdirectory(VelocitiesOnCVs)
add_subdirectory(SplitBcs)
add_subdirectory(GMSHreader)
add_subdirectory(FLMLreader)

file(GLOB TARGETS */*.dylib)

file(COPY ${TARGETS} DESTINATION plugins)
