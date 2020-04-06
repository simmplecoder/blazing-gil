foreach(example 
    sobel_gradient
    harris)

    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} blazing-gil gil_dependencies)
endforeach()