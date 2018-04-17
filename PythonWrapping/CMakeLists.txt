include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../ShowCVs ../GMSHreader ../GMSHwriter ../SplitBcs ../VelocitiesOnCVs ../MergePointFilter)

project(vtkPlugins)

set(FILE_SRCS 
   ../GMSHwriter/vtkGmshWriter.cxx
   ../ShowCVs/vtkShowCVs.cxx
   ../GMSHreader/vtkGmshReader.cxx
   ../SplitBcs/vtkSplitBcs.cxx
   ../VelocitiesOnCVs/vtkCVVelocities.cxx
   ../MergePointFilter/vtkMergePointFilter.cxx
)
 
    
cmake_minimum_required(VERSION 2.8.8)

set(PACKAGE_VERSION 00.00.01)

FIND_PACKAGE(VTK REQUIRED)
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE(${VTK_USE_FILE})
INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
find_program(PYTHON python)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(vtk_libs
    vtkCommonExecutionModel
    vtkCommonCore
    vtkFiltersCore
    vtkParallelCore
    vtkIOLegacy
    vtkIOParallel
    vtkImagingCore
    vtkFiltersGeometry)
#    vtkFiltering
#    vtkIO
#    vtkGraphics)

#set(VTK_MODULES_USED vtkCommon vtkFiltering vtkIO vtkGraphics)

link_directories(${VTK_DIR})
include_directories("${PYTHON_INCLUDE_DIRS}")

add_library(vtkPlugins SHARED ${FILE_SRCS})
target_link_libraries(vtkPlugins ${vtk_libs})

# Generate wrapper code
vtk_wrap_python3(vtkPluginsPython KitPython_SRCS "${FILE_SRCS}")
    
add_library(vtkPluginsPythonD ${KitPython_SRCS} ${Kit_PYTHON_EXTRA_SRCS})
target_link_libraries(vtkPluginsPythonD  vtkPlugins ${KIT_PYTHON_LIBS})   

IF(KIT_PYTHON_DEPS)
  ADD_DEPENDENCIES(vtkPluginsPythonD ${KIT_PYTHON_DEPS} ${VTK_PYTHON_LIBRARIES})
ENDIF(KIT_PYTHON_DEPS)

python_add_module(vtkPluginsPython vtkPluginsPythonInit.cxx)
    
#target_link_libraries(vtkPluginsPythonD         
#    ${VTK_LIBRARIES}
#    vtkPythonCore 
#    ${VTK_PYTHON_LIBRARIES} ${PYTHON_LIBRARY})
    
#add_library(vtkPlugins/vtkPluginsPython MODULE ${vtkPluginsPython_SRCS} vtkPluginsPythonInit.cxx)
    
set(VTK_PYTHOND_LIBS)
foreach(TMP_LIB ${vtk_libs})
     set(VTK_PYTHOND_LIBS ${VTK_PYTHOND_LIBS} /opt/local/lib/lib${TMP_LIB}Python27D-7.1.dylib)
endforeach()    
    
target_link_libraries(vtkPluginsPython ${PYTHON_LIBRARIES} /opt/local/lib/libvtkWrappingPython27Core-7.1.dylib vtkPluginsPythonD ${VTK_PYTHOND_LIBS})

#set_target_properties(vtkPlugins/vtkPluginsPython PROPERTIES PREFIX "")


set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")	
set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/vtkPlugins/__init__.py")
set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

configure_file(${SETUP_PY_IN} ${SETUP_PY})

add_custom_command(OUTPUT ${OUTPUT}
                  COMMAND ${PYTHON} ${SETUP_PY} build
                  COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                  DEPENDS ${DEPS})

add_custom_target(target ALL DEPENDS ${OUTPUT})

install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")
