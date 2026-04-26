cmake_minimum_required(VERSION 3.27)
project(sol2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(sol2 main.cpp
        Data.h
        BucketQueue.h
        ScheduleEvaluator.h
        GeneticAlgorithm.h
        MutationGenerator.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,16777216")

target_compile_options(sol2 PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native>
)