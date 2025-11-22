cmake_minimum_required(VERSION 3.20)
project(problem_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(problem_manager
  src/main.cpp
)

# Para que pueda hacer #include "httplib.h" y "json.hpp"
target_include_directories(problem_manager PRIVATE third_party)

if (MSVC)
  target_compile_definitions(problem_manager PRIVATE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
  target_link_libraries(problem_manager PRIVATE ws2_32)
endif()
