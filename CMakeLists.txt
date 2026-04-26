cmake_minimum_required(VERSION 4.1)
project(inara)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio
        GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)

find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)

add_executable(inara main.cpp)

target_compile_definitions(inara PUBLIC ASIO_STANDALONE)

target_include_directories(inara PUBLIC ${asio_SOURCE_DIR}/asio/include)

target_link_libraries(inara PUBLIC Threads::Threads PostgreSQL::PostgreSQL)
