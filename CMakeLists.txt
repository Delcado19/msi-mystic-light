cmake_minimum_required(VERSION 3.21)
project(MsiMysticLightApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt's CMake integration needs these on for moc/uic/rcc to run automatically.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets Test REQUIRED)
find_package(hidapi CONFIG REQUIRED)

add_executable(MsiMysticLightApp src/main.cpp)
target_link_libraries(MsiMysticLightApp PRIVATE Qt6::Core Qt6::Widgets hidapi::hidapi)

enable_testing()
add_subdirectory(tests)
