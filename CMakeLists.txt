cmake_minimum_required(VERSION 3.14)

if(UNIX)
    project(compression)

    # Set standart
    set(CMAKE_CXX_STANDARD 17)

    enable_testing()

    add_subdirectory(test)
    add_subdirectory(compression)
    add_subdirectory(app)
else()
    message(FATAL_ERROR "Currently compression library is not supporting: ${CMAKE_SYSTEM_NAME}")
endif()