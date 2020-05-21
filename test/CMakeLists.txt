find_package(Catch2 REQUIRED)

include(Catch)

add_executable(test_target 
    remap_test.cpp 
    to_matrix_channeled_test.cpp 
    rm_cvref_test.cpp 
    pixel_to_vector_test.cpp 
    vector_to_pixel_test.cpp 
    to_image_test.cpp 
    as_matrix_test.cpp
    as_matrix_channeled_test.cpp
    true_channel_type_test.cpp)
target_link_libraries(test_target PRIVATE Catch2::Catch2 blazing-gil)
catch_discover_tests(test_target)
