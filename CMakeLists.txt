cmake_minimum_required(VERSION 3.15)
project(blazing-gil)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.72 
  COMPONENTS
    filesystem
  REQUIRED)

### setup dependencies for blazing-gil

### Intel-MKL
if (NOT DEFINED ENV{MKLROOT})
  message(FATAL_ERROR "please set mkl root")
endif()

find_package(BLAS REQUIRED)
add_library(mkl_blas INTERFACE)
target_link_libraries(mkl_blas INTERFACE ${BLAS_LIBRARIES})
target_include_directories(mkl_blas INTERFACE $ENV{MKLROOT}/include)

### blaze
find_package(blaze REQUIRED)

### GIL dependencies - jpeg, png, tiff, tiffxx, Boost::filesystem, Boost::headers (headers)
add_library(GIL INTERFACE)

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)

if (TIFF_LIBRARY)
  set(TIFFXX_NAMES tiffxx)
  foreach(name ${TIFFXX_NAMES})
    list(APPEND TIFFXX_NAMES_DEBUG "${name}d")
  endforeach()
  find_library(TIFFXX_LIBRARY_RELEASE NAMES ${TIFFXX_NAMES})
  find_library(TIFFXX_LIBRARY_DEBUG NAMES ${TIFFXX_NAMES_DEBUG})
  find_path(TIFFXX_INCLUDE_DIR NAMES tiffio.hxx)
  include(SelectLibraryConfigurations)
  select_library_configurations(TIFFXX)
  mark_as_advanced(TIFFXX_LIBRARY_RELEASE TIFFXX_LIBRARY_DEBUG)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(TIFFXX REQUIRED_VARS TIFFXX_LIBRARY TIFFXX_INCLUDE_DIR)
  target_include_directories(GIL INTERFACE ${TIFFXX_INCLUDE_DIR})
  target_link_libraries(GIL INTERFACE ${TIFFXX_LIBRARY})
endif()

target_include_directories(GIL
  INTERFACE
    ${JPEG_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR})

target_link_libraries(GIL
  INTERFACE
    ${JPEG_LIBRARIES}
    PNG::PNG
    TIFF::TIFF
    Boost::headers
    Boost::filesystem)

### create the library target - blazing-gil
add_library(blazing-gil INTERFACE)
target_include_directories(blazing-gil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(blazing-gil INTERFACE GIL mkl_blas)


### dependencies for examples
add_library(cli11 INTERFACE)
target_include_directories(cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cli11)
add_subdirectory(example)