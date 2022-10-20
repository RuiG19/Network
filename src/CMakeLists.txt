cmake_minimum_required(VERSION 2.8)

project(NetworkApp)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")

# Boost
find_package( Boost 1.55 COMPONENTS system thread filesystem REQUIRED )
include_directories(SYSTEM ${Boost_INCLUDE_DIR} logger utils server client )

file(GLOB CLIENT_SRCS client/*.cpp)
add_library(${PROJECT_NAME}_Client ${CLIENT_SRCS})
file(GLOB SERVER_SRCS server/*.cpp)
add_library(${PROJECT_NAME}_Server ${SERVER_SRCS})
file(GLOB LOGGER_SRCS logger/*.cpp)
add_library(${PROJECT_NAME}_Logger ${LOGGER_SRCS})

add_executable(${PROJECT_NAME}
    main.cpp
)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_Client ${PROJECT_NAME}_Server ${PROJECT_NAME}_Logger)

