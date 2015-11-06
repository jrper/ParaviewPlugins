include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../ShowCVs)

project(vtk_plugins)

set(FILE_SRCS 
   ../ShowCVs/vtkShowCVs.cxx ../ShowCVs/vtkShowCVs.h)
    
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

add_library(vtk_plugins SHARED ${FILE_SRCS})
target_link_libraries(vtk_plugins ${vtk_libs})

# Generate wrapper code
vtk_wrap_python3(vtk_pluginsPython vtk_pluginsPython_SRCS ${FILE_SRCS})
    
add_library(vtk_pluginsPythonD ${vtk_pluginsPython_SRCS})
target_link_libraries(vtk_pluginsPythonD  vtk_plugins)   
    
#target_link_libraries(vtk_pluginsPythonD         
#    ${VTK_LIBRARIES}
#    vtkPythonCore 
#    ${VTK_PYTHON_LIBRARIES} ${PYTHON_LIBRARY})
    
add_library(vtk_pluginsPython MODULE ${vtk_pluginsPython_SRCS} vtk_pluginsPythonInit.cxx)
    
set(VTK_PYTHOND_LIBS)
foreach(TMP_LIB ${VTK_MODULES_USED})
     set(VTK_PYTHOND_LIBS ${VTK_PYTHOND_LIBS} ${TMP_LIB}PythonD)
endforeach()    
    
target_link_libraries(vtk_pluginsPython vtk_pluginsPythonD ${VTK_PYTHOND_LIBS})

set_target_properties(vtk_pluginsPython PROPERTIES PREFIX "")
