cmake_minimum_required(VERSION 3.25)
project(OSSIM C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

add_executable(ossim ossim.c queue.c scheduler.c)

add_executable(application application.c burst_queue.c)
