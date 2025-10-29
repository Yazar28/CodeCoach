cmake_minimum_required(VERSION 3.20)
project(evaluator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(evaluator
  src/main.cpp
)

target_include_directories(evaluator PRIVATE third_party)

if (MSVC)
  target_compile_definitions(evaluator PRIVATE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
  target_link_libraries(evaluator PRIVATE ws2_32)
endif()
