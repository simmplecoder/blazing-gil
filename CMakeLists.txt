cmake_minimum_required(VERSION 3.15)
project(blazing-gil)

set(CMAKE_CXX_STANDARD 17)

option(BOOST_INCLUDE_DIR "set this to your boost root folder" "")

if (${BOOST_INCLUDE_DIR} EQUAL "")
  message(FATAL_ERROR "please set boost include dir")
endif()

find_package(blaze REQUIRED)

add_library(gil_io_dependencies INTERFACE)

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)

target_link_libraries(gil_io_dependencies INTERFACE ${JPEG_LIBRARIES} PNG::PNG TIFF::TIFF)
target_include_directories(gil_io_dependencies INTERFACE ${JPEG_INCLUDE_DIR})

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
  target_include_directories(gil_io_dependencies INTERFACE ${TIFFXX_INCLUDE_DIR})
  target_link_libraries(gil_io_dependencies INTERFACE ${TIFFXX_LIBRARY})
endif()

target_include_directories(gil_io_dependencies
  INTERFACE
    ${JPEG_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR})

target_link_libraries(gil_io_dependencies
  INTERFACE
    ${JPEG_LIBRARIES}
    PNG::PNG
    TIFF::TIFF)


add_library(blazing-gil INTERFACE)
target_include_directories(blazing-gil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(blazing-gil INTERFACE gil_io_dependencies)

add_executable(program main.cpp)
target_link_libraries(program PRIVATE blazing-gil) # ${JPEG_LIBRARIES} PNG::PNG TIFF::TIFF blaze::blaze
# target_include_directories(program PUBLIC ${BOOST_INCLUDE_DIR} ${JPEG_INCLUDE_DIR})
