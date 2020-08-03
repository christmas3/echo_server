cmake_minimum_required(VERSION 3.5)

project(echo_server LANGUAGES CXX)

find_package(gflags REQUIRED)

set(PROJ_EXEC echo_server)
set(PROJ_LIBS ev gflags glog)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB PROJ_CPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB PROJ_HEADER RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_executable(${PROJ_EXEC} ${PROJ_CPP} ${PROJ_HEADER})

target_link_libraries(${PROJ_EXEC} ${PROJ_LIBS})
