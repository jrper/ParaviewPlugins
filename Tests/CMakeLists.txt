 if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
 endif(COMMAND cmake_policy)

FIND_PACKAGE( PkgConfig REQUIRED)

FIND_PACKAGE(ParaView REQUIRED)

include_directories(../MergePointFilter ../SplitBcs)

add_executable(test-paraview-plugins EXCLUDE_FROM_ALL TestParaviewPlugins.cxx
  ../ShowCVs/vtkShowCVs.cxx)

cmake_minimum_required(VERSION 2.8)

pkg_search_module(CPPUNIT cppunit)
FIND_LIBRARY(CPPUNIT_LIB cppunit)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../showCVs ${CMAKE_CURRENT_SOURCE_DIR}/../GMSHReaser ${CMAKE_CURRENT_SOURCE_DIR}/../GmshWRiter ${ParaView_LIB_DIR})
include_directories(${CPPUNIT_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

ADD_TEST(NAME test-paraview-plugins COMMAND "${PROJECT_OUTPUT_TEST_DIR}/test-paraview-plugins")

target_link_libraries(
    test-paraview-plugins
    ${CPPUNIT_LIB}
    ${CMAKE_DL_LIBS}
    showCVs
    GMSHreader GMSHwriter MergePointFilter
)

