cmake_host_system_information(RESULT max_cores QUERY NUMBER_OF_LOGICAL_CORES)

add_executable(core_test core_test.cpp)
target_link_libraries(core_test pebbl)
add_test(NAME core_test_serial COMMAND core_test)
if(enable_mpi)
  add_test(NAME core_test_MPI_2 COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 2 core_test)
  set_tests_properties(core_test_MPI_2 PROPERTIES PROCESSORS 2)
endif()

add_executable(knapMPS knapMPS.cpp parKnapsack.cpp serialKnapsack.cpp)
target_link_libraries(knapMPS pebbl)

add_executable(Docking Docking.cpp)
target_link_libraries(Docking pebbl)

add_executable(knapsack knapsack.cpp parKnapsack.cpp serialKnapsack.cpp)
target_link_libraries(knapsack pebbl)
if(knapsack_test_dir)
  add_test(NAME Knapsack_scor1k.3_serial COMMAND knapsack ${knapsack_test_dir}/scor1k.3)
  add_test(NAME Knapsack_test-data.1000.2_serial COMMAND knapsack ${knapsack_test_dir}/test-data.1000.2)
  if(enable_mpi)
    add_test(NAME Knapsack_scor1k.3_MPI_4 COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4 knapsack ${knapsack_test_dir}/scor1k.3)
    set_tests_properties(Knapsack_scor1k.3_MPI_4 PROPERTIES PROCESSORS 4)

    add_test(NAME Knapsack_test-data.1000.2_MPI_6 COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 6 knapsack ${knapsack_test_dir}/test-data.1000.2)
    set_tests_properties(Knapsack_test-data.1000.2_MPI_6 PROPERTIES PROCESSORS 6)
  endif()
endif()

if(enable_mpi)
  add_executable(commTest commTest.cpp parKnapsack.cpp serialKnapsack.cpp)
  target_link_libraries(commTest pebbl)
endif()

add_executable(lipshitzian lipshitzian.cpp)
target_link_libraries(lipshitzian pebbl)
add_test(NAME lipshitzian_serial COMMAND lipshitzian)
if(enable_mpi)
  add_test(NAME lipshitzian_MPI_3 COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 3 lipshitzian)
  set_tests_properties(lipshitzian_MPI_3 PROPERTIES PROCESSORS 3)
endif()

add_executable(logAnalyze logAnalyze.cpp)
target_link_libraries(logAnalyze pebbl)

add_executable(monomial monomial.cpp parMonomial.cpp serialMonomial.cpp)
target_link_libraries(monomial pebbl)
if(monomial_test_dir)
  add_test(NAME monomial_processed.cleveland.data.csv.ss35.bin.txt_serial
           COMMAND monomial ${monomial_test_dir}/processed.cleveland.data.csv.ss35.bin.txt)
  if(enable_mpi)
    add_test(NAME monomial_processed.cleveland.data.csv.ss35.bin.txt_MPI_5
             COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 5 monomial ${monomial_test_dir}/processed.cleveland.data.csv.ss35.bin.txt)
    set_tests_properties(monomial_processed.cleveland.data.csv.ss35.bin.txt_MPI_5 PROPERTIES PROCESSORS 5)
  endif()
endif()
