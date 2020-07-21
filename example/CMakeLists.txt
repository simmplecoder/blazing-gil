if (NOT USE_CONAN)
    find_package(CLI11 REQUIRED)
endif()

foreach(example 
    sobel_gradient
    harris
    hessian
    anisotropic_diffusion
    nearest_neighbor_scaling
    padding
    bilinear_interpolation_scaling
    lanczos_scaling
    matrix_channeled
    hough_line_transform)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} PRIVATE
        blazing-gil 
        $<IF:$<BOOL:${USE_CONAN}>,CONAN_PKG::CLI11,CLI11::CLI11>)
    target_compile_options(${example} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
    )
endforeach()