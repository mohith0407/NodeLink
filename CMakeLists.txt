cmake_minimum_required(VERSION 3.10)
project(myBitTorrent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/source/include)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)


# Build the Parsing Library
add_library(core_lib
    source/parsing/buffer.cpp
    source/parsing/bencode.cpp
    source/parsing/torrent.cpp

    source/tracker/transport.cpp
    source/tracker/tracker_utils.cpp
    source/tracker/tracker.cpp

    source/download/message.cpp
    source/download/file_manager.cpp
    source/download/downloader.cpp
)
target_link_libraries(core_lib OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

# Build the Test

# for parsing
# add_executable(run_tests tests/test_parsing.cpp)
# target_link_libraries(run_tests parsing_lib)


# Test for Tracker
# add_executable(test_tracker tests/test_tracker.cpp)
# target_link_libraries(test_tracker core_lib)

# Main Executable
add_executable(bittorrent_client tests/test_download.cpp)
target_link_libraries(bittorrent_client core_lib)