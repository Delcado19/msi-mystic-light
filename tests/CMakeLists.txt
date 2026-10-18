add_executable(test_scaffold scaffold/test_scaffold.cpp)
target_link_libraries(test_scaffold PRIVATE Qt6::Test)

add_test(NAME test_scaffold COMMAND test_scaffold)
