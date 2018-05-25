add_executable(core_test core_test.cpp)
target_link_libraries(core_test pebbl)

add_executable(knapMPS knapMPS.cpp parKnapsack.cpp serialKnapsack.cpp)
target_link_libraries(knapMPS pebbl)

add_executable(Docking Docking.cpp)
target_link_libraries(Docking pebbl)

add_executable(knapsack knapsack.cpp parKnapsack.cpp serialKnapsack.cpp)
target_link_libraries(knapsack pebbl)

add_executable(lipshitzian lipshitzian.cpp)
target_link_libraries(lipshitzian pebbl)

add_executable(logAnalyze logAnalyze.cpp)
target_link_libraries(logAnalyze pebbl)

add_executable(monomial monomial.cpp parMonomial.cpp serialMonomial.cpp)
target_link_libraries(monomial pebbl)
