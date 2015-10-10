cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MACOSX_RPATH 1)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

include_directories(${CPPUNIT_INC})

file(MAKE_DIRECTORY plugins)

add_subdirectory(ShowCVs)
add_subdirectory(VelocitiesOnCVs)
add_subdirectory(SplitBcs)
add_subdirectory(GMSHreader)
add_subdirectory(GMSHwriter)
add_subdirectory(FLMLreader)
add_subdirectory(TemporalXMLUnstructuredGridReader)
add_subdirectory(detectorreader)
add_subdirectory(statreader)
add_subdirectory(Tests)



add_custom_target(plugins DEPENDS ShowCVs VelocitiesOnCVs SplitBcs GMSHreader GMSHwriter FLMLreader TemporalXMLUnstructuredGridReader detectorreader statreader)

file(GLOB TARGETS */*.dylib)

file(COPY ${TARGETS} DESTINATION plugins)



