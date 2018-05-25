cmake_minimum_required(VERSION 3.5)
project(PEBBL CXX)
include(CheckIncludeFileCXX)

option(enable_mpi "Enable MPI build. Will require MPI wrappers for compilers." OFF)
option(enable_serializer "Enable serializer funtionality." OFF)
option(enable_examples "Enable example programs." ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

if(enable_mpi)
  message("MPI enabled.")
  check_include_file_cxx(mpi.h have_mpi_h)
  if(NOT have_mpi_h)
    #verify mpi is available.
    message("MPI is enabled, but compiler cannot find mpi.h.")
    message("Please provide the mpi wrappers for your compilers to CMAKE_<lang>_COMPILER.")
    message(FATAL_ERROR "Aborting configure.")
  endif()
  set(ACRO_HAVE_MPI 1)
  set(UTILIB_HAVE_MPI 1)
endif()

if(enable_serializer)
  set(HAVE_SERIALIZER 1)
endif()

add_subdirectory(src)
