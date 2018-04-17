set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

PROJECT(ParaviewPlugins)

set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 

SET(CMAKE_INSTALL_RPATH "\$ORIGIN/")


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
link_directories(${ParaView_LIB_DIR})


FIND_PACKAGE( PkgConfig REQUIRED)
pkg_search_module(CPPUNIT cppunit)
include_directories(${ParaView_INCLUDE_DIRS} ${CPPUNIT_INCLUDE_DIRS})

file(MAKE_DIRECTORY plugins)

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



add_custom_target(plugins DEPENDS ShowCVs VelocitiesOnCVs SplitBcs GMSHreader GMSHwriter FLMLreader TemporalDataReaders detectorreader statreader)

add_custom_command(TARGET plugins POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy */*${CMAKE_SHARED_LIBRARY_SUFFIX} plugins
)
