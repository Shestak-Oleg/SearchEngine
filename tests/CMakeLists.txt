project(Search_Engine_tests)

# get GTest from github
include(FetchContent)
FetchContent_Declare(
        googletest
        URL
        https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Search_Engine_tests
        index_tests.cpp
        search_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/index.cpp
        ${CMAKE_SOURCE_DIR}/src/search.cpp
)

target_include_directories(Search_Engine_tests
        PRIVATE
        ${gtest_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(Search_Engine_tests
        PRIVATE
        gtest_main
        nlohmann_json::nlohmann_json
)

