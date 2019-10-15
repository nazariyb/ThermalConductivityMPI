cmake_minimum_required(VERSION 3.10)
project(ThermalConductivityMPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "mpic++")

find_package(Boost COMPONENTS mpi filesystem REQUIRED)
find_package(PNG REQUIRED)

#SET(GCC_COVERAGE_COMPILE_FLAGS "-lboost_mpi")
#set(Boost_USE_STATIC_LIBS OFF)
include_directories(${Boost_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})

add_executable(ThermalConductivityMPI src/main.cpp src/main_config.h src/utils.cpp src/utils.h src/conf_reader.cpp src/conf_reader.h src/thread_safe_queue.h)
target_link_libraries(ThermalConductivityMPI ${Boost_LIBRARIES} ${PNG_LIBRARY})
