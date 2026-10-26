add_executable(test_suite test_fitness.cpp test_neighborhoods.cpp test_algorithms.cpp)
target_link_libraries(test_suite PRIVATE tp2_core GTest::gtest_main)
add_test(NAME test_suite COMMAND test_suite)
