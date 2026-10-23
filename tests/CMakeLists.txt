add_executable(test_msi_protocol protocol/test_msi_protocol.cpp)
target_link_libraries(test_msi_protocol PRIVATE msi_protocol Qt6::Test)
add_test(NAME msi_protocol COMMAND test_msi_protocol)
add_executable(test_hid_backend hid/test_hid_backend.cpp ../src/hid/HidApiDevice.cpp)
target_include_directories(test_hid_backend PRIVATE ../src
    "$<TARGET_PROPERTY:hidapi::hidapi,INTERFACE_INCLUDE_DIRECTORIES>")
target_compile_definitions(test_hid_backend PRIVATE HID_API_NO_EXPORT_DEFINE)
target_link_libraries(test_hid_backend PRIVATE Qt6::Test)
add_test(NAME hid_backend COMMAND test_hid_backend lifecycle failedOpen writes fakeFailures enumeration exactPath)
add_test(NAME hid_init_failure COMMAND test_hid_backend initializationFailure)
set_tests_properties(hid_init_failure PROPERTIES ENVIRONMENT "HID_TEST_INIT_FAIL=1")
add_library(incompatible_module MODULE modules/IncompatibleModule.cpp)
add_library(missing_interface_module MODULE modules/MissingInterface.cpp)
add_library(virtual_module MODULE modules/VirtualModule.cpp)
foreach(fixture IN ITEMS incompatible_module missing_interface_module virtual_module)
    target_include_directories(${fixture} PRIVATE ../src)
    target_link_libraries(${fixture} PRIVATE Qt6::Core)
endforeach()
add_executable(test_modules modules/test_modules.cpp)
target_include_directories(test_modules PRIVATE .)
target_link_libraries(test_modules PRIVATE lighting_modules Qt6::Test)
target_compile_definitions(test_modules PRIVATE
    TEST_MODULE_PATH="$<TARGET_FILE:msi_gen1_module>"
    TEST_PROFILE_PATH="${PROJECT_SOURCE_DIR}/profiles/msi-mpg-b850-edge-ti-wifi.json"
    INCOMPATIBLE_MODULE_PATH="$<TARGET_FILE:incompatible_module>"
    VIRTUAL_MODULE_PATH="$<TARGET_FILE:virtual_module>"
    MISSING_INTERFACE_PATH="$<TARGET_FILE:missing_interface_module>")
add_dependencies(test_modules msi_gen1_module incompatible_module missing_interface_module virtual_module)
add_test(NAME driver_modules COMMAND test_modules)
add_test(NAME app_scaffold COMMAND msi_mystic_light --inspect)
add_test(NAME app_invalid_module_directory COMMAND msi_mystic_light --inspect --module-dir "${CMAKE_BINARY_DIR}/nonexistent-modules")
set_tests_properties(app_invalid_module_directory PROPERTIES WILL_FAIL TRUE)
# Tests explicitly use the SDK offscreen plugin; app deployment includes only the native Windows plugin.
set_tests_properties(app_scaffold app_invalid_module_directory PROPERTIES
    TIMEOUT 15
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")

add_executable(test_components components/test_components.cpp)
target_link_libraries(test_components PRIVATE lighting_modules Qt6::Test)
target_compile_definitions(test_components PRIVATE TEST_COMPONENT_DIR="${PROJECT_SOURCE_DIR}/components")
add_test(NAME components COMMAND test_components)
add_test(NAME app_invalid_component_directory COMMAND msi_mystic_light --inspect --component-dir "${CMAKE_BINARY_DIR}/nonexistent-components")
set_tests_properties(app_invalid_component_directory PROPERTIES WILL_FAIL TRUE TIMEOUT 15
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")

add_executable(test_startup startup/test_startup.cpp)
target_link_libraries(test_startup PRIVATE lighting_startup Qt6::Test)
add_test(NAME startup_conflicts COMMAND test_startup)
set_tests_properties(startup_conflicts PROPERTIES TIMEOUT 20
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")

add_executable(test_editor editor/test_editor.cpp)
target_link_libraries(test_editor PRIVATE lighting_editor Qt6::Test)
target_compile_definitions(test_editor PRIVATE TEST_ARTIFACT_DIR="${CMAKE_BINARY_DIR}/editor-artifacts")
add_test(NAME lighting_editor COMMAND test_editor)
set_tests_properties(lighting_editor PROPERTIES TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")

add_executable(test_output output/test_output.cpp)
add_executable(test_scheduler output/test_scheduler.cpp)
foreach(output_test IN ITEMS test_output test_scheduler)
    target_link_libraries(${output_test} PRIVATE lighting_output msi_protocol Qt6::Test)
    target_compile_definitions(${output_test} PRIVATE
        TEST_MODULE_PATH="$<TARGET_FILE:msi_gen1_module>"
        TEST_PROFILE_PATH="${PROJECT_SOURCE_DIR}/profiles/msi-mpg-b850-edge-ti-wifi.json")
    add_dependencies(${output_test} msi_gen1_module)
endforeach()
add_test(NAME output_session COMMAND test_output)
add_test(NAME output_scheduler COMMAND test_scheduler)
set_tests_properties(output_session output_scheduler PROPERTIES TIMEOUT 30)

add_executable(test_output_ui output/test_output_ui.cpp)
target_link_libraries(test_output_ui PRIVATE lighting_output_ui msi_protocol Qt6::Test)
target_compile_definitions(test_output_ui PRIVATE
    TEST_MODULE_PATH="$<TARGET_FILE:msi_gen1_module>"
    TEST_PROFILE_PATH="${PROJECT_SOURCE_DIR}/profiles/msi-mpg-b850-edge-ti-wifi.json"
    TEST_ARTIFACT_DIR="${CMAKE_BINARY_DIR}/editor-artifacts")
add_dependencies(test_output_ui msi_gen1_module)
add_test(NAME output_ui COMMAND test_output_ui)
set_tests_properties(output_ui PROPERTIES TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")

if(WIN32)
    # Test executables must run without inheriting a Qt SDK PATH from the shell.
    foreach(test_target IN ITEMS test_msi_protocol test_hid_backend test_modules test_components test_startup test_editor test_output test_scheduler test_output_ui)
        add_custom_command(TARGET ${test_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${test_target}> $<TARGET_FILE_DIR:${test_target}>
            COMMAND_EXPAND_LISTS)
    endforeach()
endif()

if(WIN32)
    add_custom_command(TARGET test_editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:test_editor>/imageformats"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QJpegPlugin> $<TARGET_FILE:Qt6::QGifPlugin> "${WEBP_PLUGIN}"
            "$<TARGET_FILE_DIR:test_editor>/imageformats")
endif()

if(WIN32)
    add_test(NAME hidapi_runtime_deployment COMMAND ${CMAKE_COMMAND}
        "-DAPP_DLL=$<TARGET_FILE_DIR:msi_mystic_light>/hidapi.dll"
        "-DTEST_DLL=$<TARGET_FILE_DIR:test_output_ui>/hidapi.dll"
        "-DSOURCE_DLL=${HIDAPI_RUNTIME_DLL}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_hidapi_runtime.cmake")
endif()

add_executable(test_peripherals output/test_peripherals.cpp)
target_link_libraries(test_peripherals PRIVATE lighting_output_ui Qt6::Test)
target_compile_definitions(test_peripherals PRIVATE
    PERIPHERAL_MODULE_PATH="$<TARGET_FILE:peripherals_module>"
    PROFILE_DIRECTORY="${PROJECT_SOURCE_DIR}/profiles")
add_dependencies(test_peripherals peripherals_module)
add_test(NAME peripheral_reports COMMAND test_peripherals)
if(WIN32)
    add_custom_command(TARGET test_peripherals POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:test_peripherals> $<TARGET_FILE_DIR:test_peripherals>
        COMMAND_EXPAND_LISTS)
endif()
