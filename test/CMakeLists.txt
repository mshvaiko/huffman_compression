set(TEST_NAME compression_lib_test)

find_package(GTest REQUIRED)

set(TEST_NAME ${PROJECT_NAME}_test)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    ${TEST_NAME}
    hauffman_tree_test.cpp
)
target_link_libraries(
    ${TEST_NAME}
    libcompression
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})