cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  showCVs "0.0"
  SERVER_MANAGER_XML showCVs.xml
  SERVER_MANAGER_SOURCES showCVs.h showCVs.cxx
  )
