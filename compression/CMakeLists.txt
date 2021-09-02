set(libcompression_SRCS
    hauffman_tree.h
    hauffman_tree.cpp
    thread_pool.cpp
)
set(LIBRARY_NAME libcompression)

# Declare the library
add_library(${LIBRARY_NAME} SHARED
    ${libcompression_SRCS}
)

target_link_libraries(
    ${LIBRARY_NAME}
    tbb
)

# Specify here the include directories exported
# by this library
target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
