cmake_minimum_required(VERSION 3.22)
project(MsiMysticLightApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Needed for Qt's Q_OBJECT/moc code generation (used by the scaffold test).
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets OpenGLWidgets Svg Test REQUIRED)
find_package(hidapi CONFIG REQUIRED)
# Preview widgets use Qt's OpenGL paint engine for texture scaling/compositing.
link_libraries(Qt6::OpenGLWidgets Qt6::Svg)

add_executable(MsiMysticLightApp src/ui/studio.qrc src/main.cpp src/hid/HidApiDevice.cpp src/protocol/MsiProtocol.cpp src/protocol/RoccatVulcanProtocol.cpp src/protocol/RoccatKoneProtocol.cpp src/protocol/RazerBasiliskProtocol.cpp src/protocol/SteelSeriesApex3Protocol.cpp src/device/Component.cpp src/device/Channel.cpp src/device/MsiMotherboardDriver.cpp src/device/RoccatVulcanDriver.cpp src/device/RoccatKoneDriver.cpp src/device/RazerBasiliskDriver.cpp src/device/SteelSeriesApex3Driver.cpp src/effects/StaticColorEffect.cpp src/effects/RainbowWaveEffect.cpp src/effects/BreathingEffect.cpp src/effects/RadialRainbowEffect.cpp src/effects/SpiralEffect.cpp src/effects/CometEffect.cpp src/effects/MarqueeEffect.cpp src/effects/SwirlCirclesEffect.cpp src/canvas/Canvas.cpp src/settings/Settings.cpp src/ui/TrayApp.cpp src/ui/LightingCanvasWidget.cpp src/ui/LightingStudioWindow.cpp src/platform/win/AutostartWin.cpp)
target_include_directories(MsiMysticLightApp PRIVATE src)
target_link_libraries(MsiMysticLightApp PRIVATE Qt6::Core Qt6::Widgets hidapi::hidapi)

if(WIN32)
    # Direct EXE launches need Qt DLLs and the Windows platform plugin beside
    # the app; linking Qt alone only supports launches with Qt on PATH.
    get_target_property(qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(qt_bin_dir "${qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${qt_bin_dir}" REQUIRED)
    add_custom_command(TARGET MsiMysticLightApp POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations
            "$<TARGET_FILE:MsiMysticLightApp>"
        VERBATIM)
endif()


enable_testing()
add_subdirectory(tests)
