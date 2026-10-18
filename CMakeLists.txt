cmake_minimum_required(VERSION 3.22)
project(MsiMysticLightApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Needed for Qt's Q_OBJECT/moc code generation (used by the scaffold test).
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Widgets Test REQUIRED)
find_package(hidapi CONFIG REQUIRED)

add_executable(MsiMysticLightApp src/main.cpp src/hid/HidApiDevice.cpp src/protocol/MsiProtocol.cpp src/device/Component.cpp src/device/Channel.cpp src/device/MsiMotherboardDriver.cpp src/effects/StaticColorEffect.cpp src/effects/RainbowWaveEffect.cpp)
target_include_directories(MsiMysticLightApp PRIVATE src)
target_link_libraries(MsiMysticLightApp PRIVATE Qt6::Core Qt6::Widgets hidapi::hidapi)

enable_testing()
add_subdirectory(tests)
