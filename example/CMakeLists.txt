foreach(example 
    sobel_gradient
    harris
    hessian
    anisotropic_diffusion
    nearest_neighbor_scaling
    padding
    bilinear_interpolation_scaling)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} blazing-gil gil_dependencies cli11 blaze::blaze)
endforeach()