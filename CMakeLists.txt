cmake_minimum_required(VERSION 3.21)
project(MsiMysticLightApp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Test)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(hidapi REQUIRED)
get_target_property(QT_CORE_TYPE Qt6::Core TYPE)
if(QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "This project requires dynamically linked Qt.")
endif()

add_library(msi_protocol src/protocol/MsiProtocol.cpp)
target_include_directories(msi_protocol PUBLIC src)

add_library(msi_hid src/hid/HidApiDevice.cpp)
target_include_directories(msi_hid PUBLIC src)
target_link_libraries(msi_hid PRIVATE hidapi::hidapi)

add_library(lighting_modules src/modules/DeviceProfile.cpp src/modules/ModuleRegistry.cpp
    src/components/ComponentCatalog.cpp src/discovery/HardwareDiscovery.cpp)
target_include_directories(lighting_modules PUBLIC src)
target_link_libraries(lighting_modules PUBLIC Qt6::Core)

set_target_properties(msi_protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(msi_gen1_module MODULE modules/msi-gen1/MsiModule.cpp)
target_include_directories(msi_gen1_module PRIVATE src)
target_link_libraries(msi_gen1_module PRIVATE Qt6::Core msi_protocol)
set_target_properties(msi_gen1_module PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/$<CONFIG>/profiles/msi-mpg-b850-edge-ti-wifi.json"
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/profiles/msi-mpg-b850-edge-ti-wifi.json")

file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/$<CONFIG>/components/generic-strip-8.json"
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/components/generic-strip-8.json")

add_library(lighting_startup src/startup/ConflictProcesses.cpp src/startup/ConflictDialog.cpp)
target_include_directories(lighting_startup PUBLIC src)
target_link_libraries(lighting_startup PUBLIC Qt6::Widgets)
if(WIN32)
    target_link_libraries(lighting_startup PRIVATE user32)
endif()

add_library(lighting_editor src/editor/LightingDocument.cpp src/editor/LightingCanvas.cpp src/editor/LightingWindow.cpp)
target_include_directories(lighting_editor PUBLIC src)
target_link_libraries(lighting_editor PUBLIC lighting_modules Qt6::Widgets)

add_executable(msi_mystic_light src/main.cpp)
set_target_properties(msi_mystic_light PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
target_link_libraries(msi_mystic_light PRIVATE Qt6::Widgets lighting_modules msi_hid lighting_startup lighting_editor)
add_dependencies(msi_mystic_light msi_gen1_module)
if(WIN32)
    # HIDAPI is an imported .lib; CMake cannot infer its sibling runtime DLL.
    get_filename_component(HIDAPI_LIBRARY_DIR "${hidapi_LIBRARY}" DIRECTORY)
    find_file(HIDAPI_RUNTIME_DLL NAMES hidapi.dll HINTS "${HIDAPI_LIBRARY_DIR}" "${HIDAPI_LIBRARY_DIR}/../bin")
    if(NOT HIDAPI_RUNTIME_DLL)
        message(FATAL_ERROR "Set HIDAPI_RUNTIME_DLL to the HIDAPI DLL matching hidapi_LIBRARY")
    endif()
    add_custom_command(TARGET msi_mystic_light POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HIDAPI_RUNTIME_DLL}" $<TARGET_FILE_DIR:msi_mystic_light>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:msi_mystic_light> $<TARGET_FILE_DIR:msi_mystic_light>
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:msi_mystic_light>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>
            "$<TARGET_FILE_DIR:msi_mystic_light>/platforms"
        COMMAND_EXPAND_LISTS)
endif()

enable_testing()
add_subdirectory(tests)
