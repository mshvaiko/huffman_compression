set(APP_SOURCES
    main.cpp
)

set(APP_NAME ${PROJECT_NAME}_app)

add_executable(${APP_NAME}
    ${APP_SOURCES}
)

target_link_libraries(
    ${APP_NAME}
    libcompression
)

install(TARGETS ${APP_NAME} DESTINATION bin)
