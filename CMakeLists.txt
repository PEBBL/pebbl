cmake_minimum_required(VERSION 3.5)
project(PEBBL CXX)
include(CMakePackageConfigHelpers)

set(include_install_dir "include" CACHE PATH "Relative location where to install headers.")
set(pebbl_header_install_dir ${include_install_dir}/pebbl)
set(lib_install_dir "lib" CACHE PATH "Relative location where to install libraries.")
set(package_install_dir ${lib_install_dir}/pebbl)

option(enable_mpi "Enable MPI build. Will require MPI wrappers for compilers." OFF)
option(enable_serializer "Enable serializer funtionality." OFF)
option(enable_examples "Enable example programs." ON)
option(enable_validation "Enable validation features." OFF)

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Set by default in PEBBL" FORCE)
endif()

if(enable_examples)
 enable_testing()
endif()

if(enable_mpi)
  message("MPI enabled.")
  find_package(MPI REQUIRED)

  include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

  set(ACRO_HAVE_MPI 1)
  set(UTILIB_HAVE_MPI 1)
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type_lower)
if(build_type_lower MATCHES debug)
  message("Debug mode enabled.")
  set(UTILIB_YES_DEBUGPR 1)
endif()

if(enable_validation)
  set(ACRO_VALIDATING 1)
  set(UTILIB_VALIDATING 1)
  set(UTILIB_HAVE_EXECINFO_H 1)
endif()

if(enable_serializer)
  set(HAVE_SERIALIZER 1)
endif()

if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/packages/pebbl/etc/monomial)
  set(monomial_test_dir ${CMAKE_CURRENT_SOURCE_DIR}/packages/pebbl/etc/monomial)
endif()
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/packages/pebbl/etc/knapsack/problems)
  set(knapsack_test_dir ${CMAKE_CURRENT_SOURCE_DIR}/packages/pebbl/etc/knapsack/problems)
endif()

add_subdirectory(src)

configure_package_config_file(cmake/pebbl-config.cmake.in pebbl-config.cmake
                              INSTALL_DESTINATION ${package_install_dir}
                              PATH_VARS include_install_dir)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pebbl-config.cmake DESTINATION ${package_install_dir})
