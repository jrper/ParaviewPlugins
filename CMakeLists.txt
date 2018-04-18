set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

PROJECT(ParaviewPlugins)

set(CMAKE_MACOSX_RPATH TRUE)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, use the install RPATH 
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 

if (UNIX)
  if (APPLE)
    SET(CMAKE_INSTALL_RPATH "@loader_path/")
  elseif (APPLE)
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")
  endif (APPLE)
endif (UNIX)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
link_directories(${ParaView_LIB_DIR})

FIND_PACKAGE( PkgConfig REQUIRED)
pkg_search_module(CPPUNIT cppunit)
include_directories(${ParaView_INCLUDE_DIRS} ${CPPUNIT_INCLUDE_DIRS})

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

add_subdirectory(ShowCVs)
add_subdirectory(VelocitiesOnCVs)
add_subdirectory(SplitBcs)
add_subdirectory(GMSHreader)
add_subdirectory(GMSHwriter)
#add_subdirectory(FLMLreader)
add_subdirectory(TemporalDataReaders)
add_subdirectory(detectorreader)
add_subdirectory(statreader)
add_subdirectory(BoundingSurface)
add_subdirectory(MergePointFilter)
#add_subdirectory(Supermesh)
add_subdirectory(ConsistentInterpolation)
add_subdirectory(SpatialTemporalInterpolation)
add_subdirectory(Tests)


