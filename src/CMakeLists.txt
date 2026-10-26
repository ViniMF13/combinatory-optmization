add_library(tp2_core 
    AssetsMaintainance.cpp
    GreedyDescent.cpp
    SimulatedAnnealing.cpp
    GeneticAlgorithm.cpp
)
target_include_directories(tp2_core PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tp2_main main.cpp)
target_link_libraries(tp2_main PRIVATE tp2_core)
