add_executable(test_msi_protocol protocol/test_msi_protocol.cpp)
target_link_libraries(test_msi_protocol PRIVATE msi_protocol Qt6::Test)
add_test(NAME msi_protocol COMMAND test_msi_protocol)
add_test(NAME app_scaffold COMMAND msi_mystic_light)
# Build-tree executables have no deployed plugins or qt.conf yet.
set_tests_properties(app_scaffold PROPERTIES
    TIMEOUT 15
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>;QT_FORCE_STDERR_LOGGING=1")
