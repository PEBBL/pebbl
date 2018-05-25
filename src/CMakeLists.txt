include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pebbl_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/pebbl_config.h)

add_subdirectory(pebbl)
