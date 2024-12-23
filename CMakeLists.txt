cmake_minimum_required(VERSION 3.10)

project(SuperServer CXX)

# print PROJECT_SOURCE_DIR :/home/huafeng/SuperServer
# message(STATUS "Project source directory: ${PROJECT_SOURCE_DIR}")

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -g")

# Set the output directory for binary files and ensure it exists
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src/buffer)
include_directories(${PROJECT_SOURCE_DIR}/src/http)
include_directories(${PROJECT_SOURCE_DIR}/src/log)
include_directories(${PROJECT_SOURCE_DIR}/src/pool)
include_directories(${PROJECT_SOURCE_DIR}/src/server)
include_directories(${PROJECT_SOURCE_DIR}/src/time)
include_directories(${PROJECT_SOURCE_DIR}/src/main)

# Add the executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/main/runner.cpp
    ${PROJECT_SOURCE_DIR}/src/buffer/buffer.cpp
    ${PROJECT_SOURCE_DIR}/src/http/http_conn.cpp
    ${PROJECT_SOURCE_DIR}/src/http/http_request.cpp
    ${PROJECT_SOURCE_DIR}/src/http/http_response.cpp
    ${PROJECT_SOURCE_DIR}/src/log/log.cpp
    ${PROJECT_SOURCE_DIR}/src/pool/sqlconnpool.cpp
    ${PROJECT_SOURCE_DIR}/src/pool/thread_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/server/epoller.cpp
    ${PROJECT_SOURCE_DIR}/src/server/server.cpp
    ${PROJECT_SOURCE_DIR}/src/time/heap_timer.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    pthread
    mysqlclient
)


