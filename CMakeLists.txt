cmake_minimum_required (VERSION 3.0.2)
project (Comet)

# Common
include_directories(./include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -O3 -DMEMORY_INTERFACE=CacheMemory ")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

# Catapult targets
add_executable(comet.sim
							./src/core.cpp
							./src/elfFile.cpp
							./src/main.cpp
							./src/riscvISA.cpp
							./src/basic_simulator.cpp)
set_target_properties(comet.sim PROPERTIES COMPILE_FLAGS "-D__CATAPULT__")
target_include_directories(comet.sim PUBLIC ./include/ac_types/include)

add_executable(atomicTests
							./src/core.cpp
							./src/atomicTest.cpp
							./src/elfFile.cpp
							./src/riscvISA.cpp)
set_target_properties(atomicTests PROPERTIES COMPILE_FLAGS "-D__CATAPULT__")
target_include_directories(atomicTests PUBLIC ./include/ac_types/include)

# Vivado targets
add_executable(comet.sim.viva
							./src/core.cpp
							./src/elfFile.cpp
							./src/main.cpp
							./src/riscvISA.cpp
							./src/basic_simulator.cpp)
set_target_properties(comet.sim.viva PROPERTIES COMPILE_FLAGS "-D__VIVADO__")
target_include_directories(comet.sim.viva PUBLIC ./include/ap_types/include)

add_executable(atomicTests.viva
							./src/core.cpp
							./src/atomicTest.cpp
							./src/elfFile.cpp
							./src/riscvISA.cpp)
set_target_properties(atomicTests.viva PROPERTIES COMPILE_FLAGS "-D__VIVADO__")
target_include_directories(atomicTests.viva PUBLIC ./include/ap_types/include)


