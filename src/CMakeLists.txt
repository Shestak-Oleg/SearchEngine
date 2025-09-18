add_executable(Search_Engine
        main.cpp
        converter.cpp
        index.cpp
        search.cpp
)

target_include_directories(Search_Engine
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

set(FILES_DIR "${CMAKE_SOURCE_DIR}/resource")
add_custom_command(TARGET Search_Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${FILES_DIR}"
        "$<TARGET_FILE_DIR:Search_Engine>/resource"
)

target_link_libraries(Search_Engine
        PRIVATE
        nlohmann_json::nlohmann_json
)

add_custom_command(TARGET Search_Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config/config.json
        $<TARGET_FILE_DIR:Search_Engine>
)
add_custom_command(TARGET Search_Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config/requests.json
        $<TARGET_FILE_DIR:Search_Engine>
)