include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../ShowCVs ../GMSHreader ../GMSHwriter ../SplitBcs ../VelocitiesOnCVs)

project(vtkPlugins)

set(FILE_SRCS 
   ../GMSHwriter/vtkGmshWriter.cxx
   ../ShowCVs/vtkShowCVs.cxx
   ../GMSHreader/vtkGmshReader.cxx
   ../SplitBcs/vtkSplitBcs.cxx
   ../VelocitiesOnCVs/vtkCVVelocities.cxx
)
 
    
cmake_minimum_required(VERSION 2.8.8)


FIND_PACKAGE(VTK REQUIRED)
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE(${VTK_USE_FILE})
INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(vtk_libs
    vtkCommon
    vtkFiltering
    vtkIO
    vtkGraphics)

set(VTK_MODULES_USED vtkCommon vtkFiltering vtkIO vtkGraphics)

link_directories(${VTK_DIR})
include_directories("${PYTHON_INCLUDE_DIRS}")

add_library(vtkPlugins SHARED ${FILE_SRCS})
target_link_libraries(vtkPlugins ${vtk_libs})

# Generate wrapper code
vtk_wrap_python3(vtkPluginsPython vtkPluginsPython_SRCS "${FILE_SRCS}")
    
add_library(vtkPluginsPythonD ${vtkPluginsPython_SRCS})
target_link_libraries(vtkPluginsPythonD  vtkPlugins)   
    
#target_link_libraries(vtkPluginsPythonD         
#    ${VTK_LIBRARIES}
#    vtkPythonCore 
#    ${VTK_PYTHON_LIBRARIES} ${PYTHON_LIBRARY})
    
add_library(vtkPluginsPython MODULE ${vtkPluginsPython_SRCS} vtkPluginsPythonInit.cxx)
    
set(VTK_PYTHOND_LIBS)
foreach(TMP_LIB ${VTK_MODULES_USED})
     set(VTK_PYTHOND_LIBS ${VTK_PYTHOND_LIBS} ${TMP_LIB}PythonD)
endforeach()    
    
target_link_libraries(vtkPluginsPython vtkPluginsPythonD ${VTK_PYTHOND_LIBS})

set_target_properties(vtkPluginsPython PROPERTIES PREFIX "")
