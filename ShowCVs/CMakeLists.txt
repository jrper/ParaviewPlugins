cmake_minimum_required(VERSION 2.8.8)

include_directories(../MergePointFilter)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  showCVs "0.0"
  SERVER_MANAGER_XML showCVs.xml
  SERVER_MANAGER_SOURCES vtkShowCVs.h vtkShowCVs.cxx
  )

find_library(MERGEPOINTFILTER MergePointFilter PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../MergePointFilter/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if (MERGEPOINTFILTER)
TARGET_LINK_LIBRARIES(showCVs ${MERGEPOINTFILTER})
else (MERGEPOINTFILTER)
add_dependencies (showCVs MergePointFilter)
TARGET_LINK_LIBRARIES(showCVs MergePointFilter)
endif (MERGEPOINTFILTER)



