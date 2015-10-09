add_executable(test-paraview-plugins EXCLUDE_FROM_ALL TestParaviewPlugins.cxx)
target_link_libraries(
    test-paraview-plugins
    cppunit
    ${CMAKE_DL_LIBS}
)
ADD_TEST(NAME test-paraview-plugins COMMAND "${PROJECT_OUTPUT_TEST_DIR}/test-paraview-plugins")
