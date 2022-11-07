cmake_minimum_required(VERSION 3.16)
project(rasp_rc_client)

set(CMAKE_CXX_STANDARD 14)
include_directories(
        /usr/local/include
)

link_directories("/usr/local/lib")

link_libraries("/usr/local/lib/libuv.a")

add_executable(rasp_rc_client main.cpp RcClient.cpp pwm.cpp ina226.cpp)

target_link_libraries(rasp_rc_client pthread dl rt wiringPi)