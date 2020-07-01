cmake_minimum_required(VERSION 3.15)
project(blazing-gil)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

option(USE_CONAN "Use Conan to install dependencies")
# option(blas_lib_name "BLAS library to use as backend for Blaze. Only Intel MKL is supported for now. The specified library must be the one Blaze is configured for" "MKL")
set(blas_lib_name "MKL" CACHE STRING "BLAS library to use as backend for Blaze. Only Intel MKL is supported for now. The specified library must be the one Blaze is configured for.")

### setup dependencies for blazing-gil

add_library(blas_library INTERFACE)

message(STATUS "Selected BLAS library: ${blas_lib_name}")

if (blas_lib_name STREQUAL "MKL")
  if (DEFINED MKLROOT)
      set(ENV{MKLROOT} ${MKLROOT})
  elseif(NOT DEFINED ENV{MKLROOT})
      message(FATAL_ERROR "Please either pass MKLROOT variable or set environment variable MKLROOT")
  endif()
  find_package(BLAS REQUIRED)
  target_link_libraries(blas_library INTERFACE ${BLAS_LIBRARIES})
  target_include_directories(blas_library INTERFACE $ENV{MKLROOT}/include)
elseif(NOT blas_lib_name)
    # do nothing
else()
  message(FATAL_ERROR "Please select one of the supported BLAS libraries (only MKL is supported for now)")
endif()

### blaze
find_package(blaze REQUIRED)

### GIL dependencies - jpeg, png, tiff, tiffxx, Boost::filesystem, Boost::headers (headers)
add_library(GIL INTERFACE)

find_package(Boost
        COMPONENTS
        filesystem
        REQUIRED)
if (USE_CONAN)
  # Download automatically, you can also just copy the conan.cmake file
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "blazing-gil: Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
      "${CMAKE_BINARY_DIR}/conan.cmake")
  endif()

  # NOTE: See RelWithDebInfo for Release builds, http://docs.conan.io/en/latest/howtos/vs2017_cmake.html
  set(_build_type_saved ${CMAKE_BUILD_TYPE})
  if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "Release")
  endif()

  include(${CMAKE_BINARY_DIR}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing)

  set(CMAKE_BUILD_TYPE ${_build_type_saved})
  unset(_build_type_saved)

  target_link_libraries(GIL
    INTERFACE
      CONAN_PKG::libjpeg
      CONAN_PKG::libpng
      CONAN_PKG::libtiff
      CONAN_PKG::spdlog
      Boost::headers
      Boost::filesystem)
else()
  find_package(JPEG REQUIRED)
  find_package(PNG REQUIRED)
  find_package(TIFF REQUIRED)
  find_package(spdlog REQUIRED)
  target_include_directories(GIL
    INTERFACE
      ${JPEG_INCLUDE_DIR})
  target_link_libraries(GIL
    INTERFACE
      ${JPEG_LIBRARIES}
      PNG::PNG
      TIFF::TIFF
      Boost::headers
      Boost::filesystem
      spdlog::spdlog)
endif()

### create the library target - blazing-gil
add_library(blazing-gil INTERFACE)
target_include_directories(blazing-gil INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(blazing-gil INTERFACE 
  GIL
  blas_library)

add_library(blazing-gil::blazing-gil ALIAS blazing-gil)


set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY ON)

### dependencies for examples
add_subdirectory(example)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  add_subdirectory(test)
endif()

include(GNUInstallDirs)

install(TARGETS blazing-gil GIL blas_library
  EXPORT blazing-gilTargets
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  DIRECTORY
    include/
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT blazing-gilTargets
  NAMESPACE blazing-gil::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/blazing-gil")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/blazing-gilConfig.cmake.in" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/blazing-gilConfig.cmake" @ONLY)
install(
    FILES cmake/blazing-gilConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/blazing-gil")