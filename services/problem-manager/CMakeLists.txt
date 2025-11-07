cmake_minimum_required(VERSION 3.20)
project(problem-manager CXX)

# (1) Toolchain de vcpkg (ruta relativa desde este CMakeLists al vcpkg del repo)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(problem-manager
    src/main.cpp
)

# (2) Drivers MongoDB
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)

target_link_libraries(problem-manager PRIVATE mongo::mongocxx_shared mongo::bsoncxx_shared)

# (3) Si usas httplib.h y json.hpp como headers locales:
target_include_directories(problem-manager PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party
)
