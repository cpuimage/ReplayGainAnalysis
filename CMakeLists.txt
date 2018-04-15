cmake_minimum_required(VERSION 3.9)
project(gain_demo)

add_executable(gain_demo main.c gain_analysis.c gain_analysis.h)
target_link_libraries(gain_demo -lm )