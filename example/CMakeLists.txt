foreach(example 
    sobel_gradient
    harris
    hessian
    anisotropic_diffusion
    nearest_neighbor_scaling
    padding
    bilinear_interpolation_scaling
    lanczos_scaling)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} blazing-gil cli11)
    target_compile_options(${example} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
    )
endforeach()