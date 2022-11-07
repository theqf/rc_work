cmake_minimum_required(VERSION 3.16)
project(rc_server)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        /usr/include
        /usr/local/include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wsign-compare -Wignored-qualifiers -Wno-unused-function -Wno-unused-variable")

file(GLOB SOURCE_A_FILES "*.c" "*.cpp" "*.cc" )
file(GLOB SOURCE_B_FILES "luasrc/*.c")

link_directories("/usr/local/lib" )
link_libraries("/usr/local/lib/libev.a")
link_libraries("/usr/local/lib/libglog.a")

add_executable(rc_server ${SOURCE_A_FILES} ${SOURCE_B_FILES})

target_link_libraries(rc_server pthread m dl)