add_executable(test_scaffold scaffold/test_scaffold.cpp)
target_link_libraries(test_scaffold PRIVATE Qt6::Test)

add_test(NAME test_scaffold COMMAND test_scaffold)

# Qt is not vcpkg-managed (unlike hidapi), so its DLLs aren't app-local
# copied next to the test binary. Prepend Qt's own bin dir to PATH for the
# test run so `ctest` works out of the box, without requiring every
# developer/CI shell to have Qt on PATH already.
set_tests_properties(test_scaffold PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
