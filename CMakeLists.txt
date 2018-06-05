cmake_minimum_required(VERSION 3.5)
project(PEBBL CXX)
include(CheckIncludeFileCXX)

option(enable_mpi "Enable MPI build. Will require MPI wrappers for compilers." OFF)
option(enable_serializer "Enable serializer funtionality." OFF)
option(enable_examples "Enable example programs." ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

if(enable_mpi)
  message("MPI enabled.")
  find_package(MPI REQUIRED)

  include_directories(${MPI_CXX_INCLUDE_PATH})

  set(ACRO_HAVE_MPI 1)
  set(UTILIB_HAVE_MPI 1)
endif()

if(enable_serializer)
  set(HAVE_SERIALIZER 1)
endif()

add_subdirectory(src)
