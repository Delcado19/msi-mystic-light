cmake_minimum_required(VERSION 3.21)
project(MsiMysticLightApp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Test)
get_target_property(QT_CORE_TYPE Qt6::Core TYPE)
if(QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "This project requires dynamically linked Qt.")
endif()

add_library(msi_protocol src/protocol/MsiProtocol.cpp)
target_include_directories(msi_protocol PUBLIC src)

add_executable(msi_mystic_light src/main.cpp)
target_link_libraries(msi_mystic_light PRIVATE Qt6::Widgets msi_protocol)

enable_testing()
add_subdirectory(tests)
