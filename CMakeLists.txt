cmake_minimum_required(VERSION 3.2) 

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(myBitTorrent
        VERSION 1.0
        DESCRIPTION "a torrenting app"
        LANGUAGES C CXX
        )

include_directories(source/include)

add_subdirectory(source)
