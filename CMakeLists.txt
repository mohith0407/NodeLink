cmake_minimum_required(VERSION 3.10)

project(BitTorrent
        VERSION 1.0
        DESCRIPTION "Simple Torrent Client"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Include the 'include' folder so we can write #include "parsing/Bnode.h"
include_directories(include)

# 2. Find OpenSSL (Required for SHA-1)
find_package(OpenSSL REQUIRED)

# 3. Find all .cpp files inside 'source/' recursively
file(GLOB_RECURSE SOURCES "source/*.cpp")

# --- TARGET 1: The Main Application ---
# (Comment this out until you have a working main.cpp, or keep it if you do)
 add_executable(bittorrent ${SOURCES} test/main.cpp)
 target_link_libraries(bittorrent OpenSSL::SSL OpenSSL::Crypto pthread)

# --- TARGET 2: The Parser Test ---
# We link the sources (Bnode.cpp, etc.) but NOT main.cpp (to avoid 'multiple main' errors)

# for parsing
# add_executable(test_parser test/TestParser.cpp ${SOURCES})

# for tracker
# add_executable(test_tracker test/TestTracker.cpp ${SOURCES})

# for download
# add_executable(test_download test/TestDownload.cpp ${SOURCES})

# Link libraries to the test
# for parsing
# target_link_libraries(test_parser OpenSSL::SSL OpenSSL::Crypto pthread)

# for tracker
# target_link_libraries(test_tracker OpenSSL::SSL OpenSSL::Crypto pthread)

# for download
# target_link_libraries(test_download OpenSSL::SSL OpenSSL::Crypto pthread)
# add_subdirectory(googletest)


# test message

# --- STEP 1 TEST ---
# add_executable(test_message test/TestMessage.cpp ${SOURCES})

# --- STEP 2 TEST ---
# add_executable(test_worker test/TestWorker.cpp ${SOURCES})
# target_include_directories(test_worker PRIVATE include)
# target_link_libraries(test_worker OpenSSL::SSL OpenSSL::Crypto pthread)

# --- STEP 3 TEST ---
add_executable(test_connection test/TestConnection.cpp ${SOURCES})
target_include_directories(test_connection PRIVATE include)
target_link_libraries(test_connection OpenSSL::SSL OpenSSL::Crypto pthread)