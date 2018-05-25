file(GLOB bb_srcs     bb/*.cpp)
file(GLOB comm_srcs   comm/*.cpp)
file(GLOB misc_srcs   misc/*.cpp)
file(GLOB pebbl_srcs  pbb/*.cpp)
file(GLOB sched_srcs  sched/*.cpp)
file(GLOB utilib_srcs utilib/*.cpp)

add_library(pebbl ${bb_srcs} ${comm_srcs} ${misc_srcs} ${pebbl_srcs} ${sched_srcs} ${utilib_srcs})


if(enable_examples)
  add_subdirectory(example)
endif()
