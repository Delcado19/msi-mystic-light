add_executable(test_scaffold scaffold/test_scaffold.cpp)
target_link_libraries(test_scaffold PRIVATE Qt6::Test)

add_test(NAME test_scaffold COMMAND test_scaffold)

# Qt is not vcpkg-managed (unlike hidapi), so its DLLs aren't app-local
# copied next to the test binary. Prepend Qt's own bin dir to PATH for the
# test run so `ctest` works out of the box, without requiring every
# developer/CI shell to have Qt on PATH already.
set_tests_properties(test_scaffold PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(test_msi_protocol protocol/test_msi_protocol.cpp ${CMAKE_SOURCE_DIR}/src/protocol/MsiProtocol.cpp)
target_include_directories(test_msi_protocol PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_msi_protocol PRIVATE Qt6::Test)

add_test(NAME test_msi_protocol COMMAND test_msi_protocol)

# Same PATH fix as test_scaffold above: Qt isn't vcpkg-managed, so its DLLs
# aren't app-local copied next to this test binary either.
set_tests_properties(test_msi_protocol PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(test_component device/test_component.cpp ${CMAKE_SOURCE_DIR}/src/device/Component.cpp)
target_include_directories(test_component PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_component PRIVATE Qt6::Test)

add_test(NAME test_component COMMAND test_component)

# Same PATH fix as test_scaffold above: Qt isn't vcpkg-managed, so its DLLs
# aren't app-local copied next to this test binary either.
set_tests_properties(test_component PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(test_channel device/test_channel.cpp ${CMAKE_SOURCE_DIR}/src/device/Channel.cpp ${CMAKE_SOURCE_DIR}/src/device/Component.cpp)
target_include_directories(test_channel PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_channel PRIVATE Qt6::Test)

add_test(NAME test_channel COMMAND test_channel)

# Same PATH fix as test_scaffold above: Qt isn't vcpkg-managed, so its DLLs
# aren't app-local copied next to this test binary either.
set_tests_properties(test_channel PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(test_msi_motherboard_driver device/test_msi_motherboard_driver.cpp ${CMAKE_SOURCE_DIR}/src/device/MsiMotherboardDriver.cpp ${CMAKE_SOURCE_DIR}/src/device/Channel.cpp ${CMAKE_SOURCE_DIR}/src/device/Component.cpp ${CMAKE_SOURCE_DIR}/src/protocol/MsiProtocol.cpp)
target_include_directories(test_msi_motherboard_driver PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_msi_motherboard_driver PRIVATE Qt6::Test)

add_test(NAME test_msi_motherboard_driver COMMAND test_msi_motherboard_driver)

# Same PATH fix as test_scaffold above: Qt isn't vcpkg-managed, so its DLLs
# aren't app-local copied next to this test binary either.
set_tests_properties(test_msi_motherboard_driver PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(test_effects effects/test_effects.cpp ${CMAKE_SOURCE_DIR}/src/effects/StaticColorEffect.cpp ${CMAKE_SOURCE_DIR}/src/effects/RainbowWaveEffect.cpp)
target_include_directories(test_effects PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_effects PRIVATE Qt6::Test)

add_test(NAME test_effects COMMAND test_effects)

# Same PATH fix as test_scaffold above: Qt isn't vcpkg-managed, so its DLLs
# aren't app-local copied next to this test binary either.
set_tests_properties(test_effects PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
