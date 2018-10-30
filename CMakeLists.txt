cmake_minimum_required(VERSION 3.12)
project(RSA)

set(CMAKE_CXX_STANDARD 17)
FIND_PACKAGE(Boost 1.67 COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_executable(RSA main.cpp)
target_link_libraries(RSA fftw3 ${Boost_LIBRARIES})
