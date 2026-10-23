cmake_minimum_required(VERSION 3.21)
project(MsiMysticLightApp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Test)
find_package(Threads REQUIRED)
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

add_library(lighting_editor src/editor/AppSession.cpp src/editor/ChannelAssignment.cpp src/editor/ChannelEditor.cpp src/editor/ComponentEditor.cpp src/editor/EffectRenderer.cpp src/editor/LightingDocument.cpp src/editor/LightingCanvas.cpp src/editor/LightingWindow.cpp)
target_include_directories(lighting_editor PUBLIC src)
target_link_libraries(lighting_editor PUBLIC lighting_modules Qt6::Widgets)

add_library(lighting_output src/output/OutputSession.cpp src/output/OutputScheduler.cpp)
target_include_directories(lighting_output PUBLIC src)
target_link_libraries(lighting_output PUBLIC lighting_editor Threads::Threads)

add_library(lighting_output_ui src/output/OutputPanel.cpp src/output/HardwareOutput.cpp)
target_include_directories(lighting_output_ui PUBLIC src)
target_link_libraries(lighting_output_ui PUBLIC lighting_output lighting_startup msi_hid)

add_executable(msi_mystic_light src/main.cpp)
set_target_properties(msi_mystic_light PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
target_link_libraries(msi_mystic_light PRIVATE Qt6::Widgets lighting_modules msi_hid lighting_startup lighting_editor lighting_output_ui)
add_dependencies(msi_mystic_light msi_gen1_module)
if(WIN32)
    # A matching SDK plugin is preferred; the documented source build supplies SDKs without ImageFormats.
    set(WEBP_PLUGIN_HINTS "${Qt6_DIR}/../../../plugins/imageformats")
    if(Qt6_VERSION VERSION_EQUAL "6.8.3")
        list(APPEND WEBP_PLUGIN_HINTS "${PROJECT_SOURCE_DIR}/build/qtimageformats/plugins/imageformats")
    endif()
    find_file(WEBP_PLUGIN_RELEASE qwebp.dll HINTS ${WEBP_PLUGIN_HINTS})
    find_file(WEBP_PLUGIN_DEBUG qwebpd.dll HINTS ${WEBP_PLUGIN_HINTS})
    if(NOT WEBP_PLUGIN_RELEASE OR NOT WEBP_PLUGIN_DEBUG)
        message(FATAL_ERROR "Install matching Qt ImageFormats plugins, or run tools/build-webp.ps1 from a VS developer shell")
    endif()
    set(WEBP_PLUGIN "$<IF:$<CONFIG:Debug>,${WEBP_PLUGIN_DEBUG},${WEBP_PLUGIN_RELEASE}>")
    add_custom_command(TARGET msi_mystic_light POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:msi_mystic_light> $<TARGET_FILE_DIR:msi_mystic_light>
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:msi_mystic_light>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>
            "$<TARGET_FILE_DIR:msi_mystic_light>/platforms"
        COMMAND_EXPAND_LISTS)
    # PNG/BMP are built into QtGui; JPEG/GIF/WebP need matching runtime plugins.
    add_custom_command(TARGET msi_mystic_light POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:msi_mystic_light>/imageformats"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QJpegPlugin> $<TARGET_FILE:Qt6::QGifPlugin> "${WEBP_PLUGIN}"
            "$<TARGET_FILE_DIR:msi_mystic_light>/imageformats"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/third_party/licenses"
            "$<TARGET_FILE_DIR:msi_mystic_light>/third_party/licenses"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md" "${PROJECT_SOURCE_DIR}/LICENSE" "${PROJECT_SOURCE_DIR}/PORTABLE_README.txt"
            "$<TARGET_FILE_DIR:msi_mystic_light>")
endif()

enable_testing()
add_subdirectory(tests)
