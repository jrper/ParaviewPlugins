cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

ADD_PARAVIEW_PLUGIN(
  BoundingSurface "0.0"
  SERVER_MANAGER_XML BoundingSurface.xml
  SERVER_MANAGER_SOURCES vtkBoundingSurface.h vtkBoundingSurface.cxx
  )

