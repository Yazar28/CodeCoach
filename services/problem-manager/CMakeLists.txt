cmake_minimum_required(VERSION 3.20)
project(problem_manager CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(problem_manager src/main.cpp)

# Headers de un solo archivo (sin vcpkg)
target_include_directories(problem_manager PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Necesario en Windows para sockets
if (MSVC)
  # Windows 10+
  target_compile_definitions(problem_manager PRIVATE
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
  target_link_libraries(problem_manager PRIVATE ws2_32)
endif()

