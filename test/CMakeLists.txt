if (USE_CONAN)
    add_library(Catch2::Catch2 ALIAS CONAN_PKG::Catch2)
else()
    find_package(Catch2 REQUIRED)
endif()

include(Catch)

add_executable(test_target remap_test.cpp to_matrix_test.cpp)
target_link_libraries(test_target PRIVATE Catch2::Catch2 blazing-gil)
catch_discover_tests(test_target)
