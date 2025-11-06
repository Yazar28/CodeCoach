cmake_minimum_required(VERSION 3.10)
project(analyzer)

set(CMAKE_CXX_STANDARD 17)

# Archivo ejecutable
add_executable(analyzer src/main.cpp)

# Incluir third_party
target_include_directories(analyzer PRIVATE third_party)
