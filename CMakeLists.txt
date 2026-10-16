cmake_minimum_required(VERSION 3.14)
project(TP2_IA CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Flags de otimização agressiva necessárias para bater o orçamento de tempo
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")

# GoogleTest para TDD da função de fitness
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
