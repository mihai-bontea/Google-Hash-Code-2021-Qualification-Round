cmake_minimum_required(VERSION 3.27)
project(sol1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(sol1 main.cpp
        Data.h
        MergeFindSet.h
        ScheduleEvaluator.h
        BucketQueue.h)

target_compile_options(sol1 PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native>
)