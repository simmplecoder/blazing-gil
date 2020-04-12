cmake_minimum_required(VERSION 3.15)
project(blazing-gil)

set(CMAKE_CXX_STANDARD 17)

option(BOOST_INCLUDE_DIR "set this to your boost root folder" "")

### setup dependencies for blazing-gil

### Intel-MKL
if (NOT DEFINED ENV{MKLROOT})
  message(FATAL_ERROR "please set mkl root")
endif()

find_package(BLAS REQUIRED)

### blaze
find_package(blaze REQUIRED)

### GIL dependencies - jpeg, png, tiff, tiffxx, boost
add_library(gil_dependencies INTERFACE)

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)

target_link_libraries(gil_dependencies INTERFACE ${JPEG_LIBRARIES} PNG::PNG TIFF::TIFF)
target_include_directories(gil_dependencies INTERFACE ${JPEG_INCLUDE_DIR})

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
  target_include_directories(gil_dependencies INTERFACE ${TIFFXX_INCLUDE_DIR})
  target_link_libraries(gil_dependencies INTERFACE ${TIFFXX_LIBRARY})
endif()

target_include_directories(gil_dependencies
  INTERFACE
    ${JPEG_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR})

target_link_libraries(gil_dependencies
  INTERFACE
    ${JPEG_LIBRARIES}
    PNG::PNG
    TIFF::TIFF)

### create the library target - blazing-gil
add_library(blazing-gil INTERFACE)
target_include_directories(blazing-gil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include $ENV{MKLROOT}/include)
target_link_libraries(blazing-gil INTERFACE gil_dependencies ${BLAS_LIBRARIES})


### dependencies for examples
add_library(cli11 INTERFACE)
target_include_directories(cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cli11)
add_subdirectory(example)