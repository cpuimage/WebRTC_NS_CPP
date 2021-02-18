cmake_minimum_required(VERSION 2.8)
project(webrtc_ns_cpp)
file(GLOB NS_SRC ns/*.cc ns/*.h ns/*.c)
add_executable(webrtc_ns_cpp main.cc ${NS_SRC})

SET(CMAKE_C_FLAGS_DEBUG "-O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-O3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftree-vectorize")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-vectorize")
SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -Wall -g -O0 -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_RELDEBINFO} -g -O3")

target_link_libraries(webrtc_ns_cpp -lm)
