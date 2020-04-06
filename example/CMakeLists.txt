foreach(example 
    sobel_gradient)

    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} blazing-gil gil_dependencies)
endforeach()