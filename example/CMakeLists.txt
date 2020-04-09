foreach(example 
    sobel_gradient
    harris
    hessian
    anisotropic_diffusion)

    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} blazing-gil gil_dependencies cli11)
endforeach()