cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  detectorreader "0.0"
  SERVER_MANAGER_XML detectorreader.xml
  SERVER_MANAGER_SOURCES detectorreader.h detectorreader.cxx
  GUI_RESOURCE_FILES detectorreaderGUI.xml
  )

FIND_LIBRARY(XML2_LIBRARY xml2)
find_path(LIBXML2_INCLUDE_DIR libxml/xpath.h
          HINTS ${PC_LIBXML_INCLUDEDIR} ${PC_LIBXML_INCLUDE_DIRS}
          PATH_SUFFIXES libxml2 )
include_directories(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(detectorreader ${XML2_LIBRARY})
