cmake_minimum_required(VERSION 3.28)
project(img_to_ascii C)

set(CMAKE_C_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(img_to_ascii main.c)
