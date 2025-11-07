cmake_minimum_required(VERSION 3.20)
project(analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(analyzer
  src/main.cpp
)

target_include_directories(analyzer PRIVATE third_party)

if (MSVC)
  target_compile_definitions(analyzer PRIVATE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
  target_link_libraries(analyzer PRIVATE ws2_32)
endif()
