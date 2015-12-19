cmake_minimum_required(VERSION 2.8.8)

project(Supermesh C CXX Fortran)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_library(JUDY_LIB Judy)
FIND_LIBRARY(SPUD_LIBRARY spud HINTS ${FLUIDITY_LIB_DIR})
find_library(SPATIAL_INDEX_LIB spatialindex HINTS ${FLUIDITY_LIB_DIR})

find_library(BLAS_LIB blas)
find_library(LAPACK_LIB lapack)
FIND_PACKAGE(PETSc REQUIRED)
FIND_PATH(PETSc_INCLUDE_DIR petscversion.h)
FIND_PATH(PETSc_PACKAGE_INCLUDE_DIR petscconf.h)

add_subdirectory(supermesh_femtools)
include_directories(supermesh_femtools ${PETSC_PACKAGE_INCLUDES} ${PETSc_INCLUDE_DIR} ${PETSc_PACKAGE_INCLUDE_DIR} )

FIND_PACKAGE(MPI REQUIRED)
include (CMakeForceCompiler)
CMAKE_FORCE_Fortran_COMPILER(mpif90 ${CMAKE_Fortran_COMPILER_ID})

set (CMAKE_Fortran_FLAGS "-ffree-line-length-none -ffixed-line-length-none -fdefault-real-8 -fdefault-double-8 -O2 -frecord-marker=4 -fPIC")

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})


ADD_PARAVIEW_PLUGIN(
  Supermesh "0.0"
  SERVER_MANAGER_XML Supermesh.xml
  SERVER_MANAGER_SOURCES vtkSupermesh.h vtkSupermesh.cxx
  			 vtkLinearSupermesh.h vtkLinearSupermesh.cxx
  			 vtkFortranSupermesh.F90
  )

target_link_libraries(Supermesh  ${LAPACK_LIB} ${BLAS_LIB} ${SPUD_LIBRARY} ${SPATIAL_INDEX_LIB} ${JUDY_LIB} ${MPI_LIBRARY} ${MPI_Fortran_LIBRARIES} supermesh_femtools)
