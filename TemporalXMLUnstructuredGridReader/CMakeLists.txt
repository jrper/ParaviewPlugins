cmake_minimum_required(VERSION 2.8.8)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(
  TemporalXMLUnstructuredGridReader "0.0"
  SERVER_MANAGER_XML TemporalXMLUnstructuredGridReader.xml
  SERVER_MANAGER_SOURCES TemporalXMLUnstructuredGridReader.h TemporalXMLUnstructuredGridReader.cxx TemporalXMLPUnstructuredGridReader.h TemporalXMLPUnstructuredGridReader.cxx TemporalXMLPolyDataReader.h TemporalXMLPolyDataReader.cxx TemporalXMLPPolyDataReader.h TemporalXMLPPolyDataReader.cxx
  GUI_RESOURCE_FILES TemporalXMLUnstructuredGridReaderGUI.xml
  )
