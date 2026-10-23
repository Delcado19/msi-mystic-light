add_executable(test_msi_protocol protocol/test_msi_protocol.cpp)
target_link_libraries(test_msi_protocol PRIVATE msi_protocol Qt6::Test)
add_test(NAME msi_protocol COMMAND test_msi_protocol)
add_executable(test_hid_backend hid/test_hid_backend.cpp ../src/hid/HidApiDevice.cpp)
target_include_directories(test_hid_backend PRIVATE ../src
    "$<TARGET_PROPERTY:hidapi::hidapi,INTERFACE_INCLUDE_DIRECTORIES>")
target_compile_definitions(test_hid_backend PRIVATE HID_API_NO_EXPORT_DEFINE)
target_link_libraries(test_hid_backend PRIVATE Qt6::Test)
add_test(NAME hid_backend COMMAND test_hid_backend lifecycle failedOpen writes fakeFailures)
add_test(NAME hid_init_failure COMMAND test_hid_backend initializationFailure)
set_tests_properties(hid_init_failure PROPERTIES ENVIRONMENT "HID_TEST_INIT_FAIL=1")
add_test(NAME app_scaffold COMMAND msi_mystic_light)
# Build-tree executables have no deployed plugins or qt.conf yet.
set_tests_properties(app_scaffold PROPERTIES
    TIMEOUT 15
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")
