 if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
 endif(COMMAND cmake_policy)

FIND_PACKAGE( PkgConfig REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

add_executable(test-paraview-plugins EXCLUDE_FROM_ALL TestParaviewPlugins.cxx
  ../ShowCVs/vtkShowCVs.cxx)

cmake_minimum_required(VERSION 2.8)

pkg_search_module(CPPUNIT cppunit)

link_directories(../plugins)
include_directories(${CPPUNIT_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

set(vtk_libs
    vtkCommon
    vtkFiltering
    vtkIO
    vtkGraphics)

target_link_libraries(
    test-paraview-plugins
    ${CPPUNIT_LIB}
    ${CMAKE_DL_LIBS}
    ${VTK_LIBS}
    showCVs
    GMSHreader GMSHwriter
)
ADD_TEST(NAME test-paraview-plugins COMMAND "${PROJECT_OUTPUT_TEST_DIR}/test-paraview-plugins")
