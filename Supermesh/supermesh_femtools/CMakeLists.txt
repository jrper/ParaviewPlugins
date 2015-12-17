cmake_minimum_required(VERSION 2.8.8)

project(supermesh_femtools C CXX Fortran)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_path(FLUIDITY_INCLUDE_DIR Judy.h)
find_library(JUDY_LIB judy)
FIND_LIBRARY(SPUD_LIBRARY spud)
find_library(SPATIAL_INDEX_LIB libspatialindex)

FIND_PACKAGE(PETSC REQUIRED)



FIND_PACKAGE(PythonLibs REQUIRED)

exec_program ("python"
      ARGS "-c 'import numpy; print numpy.get_include()'"
      OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
      RETURN_VALUE NUMPY_NOT_FOUND)
    if (NUMPY_INCLUDE_DIR MATCHES "Traceback")
    # Did not successfully include numpy
      set(NUMPY_FOUND FALSE)
    else (NUMPY_INCLUDE_DIR MATCHES "Traceback")
    # successful
      set (NUMPY_FOUND TRUE)
      set (NUMPY_INCLUDE_DIR ${NUMPY_INCLUDE_DIR} CACHE PATH "Numpy include path")
    endif (NUMPY_INCLUDE_DIR MATCHES "Traceback")

include_directories(${PETSC_PACKAGE_INCLUDES} ${PYTHON_INCLUDE_DIR} ${NUMPY_INCLUDE_DIR} ${CMAKE_SRC_DIR})

FIND_PACKAGE(MPI REQUIRED)
include (CMakeForceCompiler)
CMAKE_FORCE_Fortran_COMPILER(mpif90 ${CMAKE_Fortran_COMPILER_ID})
set ( CMAKE_C_FLAGS " -DHAVE_PYTHON -DHAVE_NUMPY")
set (CMAKE_Fortran_FLAGS "-ffree-line-length-none -ffixed-line-length-none -fdefault-real-8 -fdefault-double-8 -O2 -frecord-marker=4")

set_source_files_properties(fspud.F90 PROPERTIES COMPILE_FLAGS "-fno-default-real-8 -fno-default-double-8")

add_library(supermesh_femtools STATIC Fields_Allocates.F90 Element_set.F90 Elements.F90 Element_Numbering.F90 FUtils.F90 Debug.F90 Debug_Parameters.F90 Global_Parameters.F90 Quadrature.F90 Reference_counting.F90 Wandzura_Quadrature.F90 Grundmann_Moeller_Quadrature.F90 Vector_Tools.F90 Parallel_Tools.F90 MPI_Interfaces.F90 Polynomials.F90 Fields_Data_Types.F90 Picker_Data_Types.F90 Shape_Functions.F90 Superconvergence.F90 ieee_arithmetic_dummy.F90 Sparse_Tools.F90 Halo_Data_Types.F90 Halos_Allocates.F90 Halos_Base.F90 qsortd.F90 Halos_Debug.F90 Memory_Diagnostics.F90 fspud.f90 Data_Structures.F90 Integer_set.F90 Integer_hash_table.F90 Fields_Base.F90 Tensors.F90 Embed_Python_Fortran.F90 Halos_Repair.F90 Halos_Numbering.F90 Halos_Communications.F90 Linked_Lists.F90 Halos_Ownership.F90 Pickers_Deallocates.F90 Pickers_Base.F90 Adjacency_Lists.F90 Global_Numbering.F90 embed_python.c ieee_arithmetic_C99.c Data_structures_C.c Unittest_Tools_Cpp.cpp Pickers_Deallocates.F90 Node_Owner_Finder_Fortran.F90 C++_Debug.cpp Fields.F90 Fields_Manipulation.F90 Fields_Calculations.F90 FETools.F90 Transform_Elements.F90 CV_Faces.F90 EventCounter.F90 Parallel_Fields.F90 Supermesh.F90 Metric_Tools.F90 Unittest_tools.F90 Unify_meshes.F90 Tetrahedron_intersection.F90 Intersection_finder.F90 Node_Owner_Finder.cpp Element_Intersection.cpp eleset.cpp tet_predicate.cpp tri_predicate.c Write_GMSH.F90 GMSH_Common.F90
Wm4Command.cpp
Wm4FoundationPCH.cpp
Wm4Intersector.cpp
Wm4Intersector1.cpp
Wm4IntrQuad2Quad2.cpp
Wm4IntrTetrahedron3Tetrahedron3.cpp
Wm4IntrTriangle2Triangle2.cpp
Wm4Math.cpp
Wm4Memory.cpp
Wm4System.cpp
Wm4Vector2.cpp
Wm4Vector3.cpp)
