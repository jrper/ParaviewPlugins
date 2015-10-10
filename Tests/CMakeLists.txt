add_executable(test-paraview-plugins EXCLUDE_FROM_ALL TestParaviewPlugins.cxx)
find_library(CPPUNIT_LIB cppunit)
target_link_libraries(
    test-paraview-plugins
    ${CPPUNIT_LIB}
    ${CMAKE_DL_LIBS}
    showCVs GMSHreader GMSHwriter
)
ADD_TEST(NAME test-paraview-plugins COMMAND "${PROJECT_OUTPUT_TEST_DIR}/test-paraview-plugins")
