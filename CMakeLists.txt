cmake_minimum_required(VERSION 3.25)

project(
  cpuid-proj
  VERSION 1.0
  LANGUAGES CXX)

# setting default c++ standard
# if (NOT(CMAKE_CXX_STANDARD))
#     set(CMAKE_CXX_STANDARD 23)
# endif()
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

# -fmodules-ts
# add_compile_options(-fmodules-ts)

# add_subdirectory(src)

set(test_cpuid cpuid_exe)
add_executable(${test_cpuid})
target_sources(${test_cpuid} PRIVATE
  src/main.cpp
  src/utils.hpp src/utils.cpp
  src/cpuid.hpp)
# target_compile_definitions(${test_cpuid} PRIVATE cxx_std_23)
# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(${test_cpuid} PRIVATE fmt)

find_package(Qt6 REQUIRED COMPONENTS Core)
target_link_libraries(${test_cpuid} PRIVATE Qt6::Core)