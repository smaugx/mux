cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project(MUX CXX C)

add_definitions(
    -DTEST1   # define marco
    -DTEST2   # define marco
)

option(XENABLE_TEST3  "enable test3 marco" OFF)

if (XENABLE_TEST3) 
    add_definitions(-DTEST3)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# common compiling options
add_compile_options(
    -Wl,--no-as-needed
    -fno-strict-aliasing
    -fthreadsafe-statics
    -pthread
    #-fstack-protector-strong
    -fno-short-enums
    -fPIC
)

set(EXECUTABLE_OUTPUT_PATH ${MUX_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${MUX_BINARY_DIR}/lib)


message(STATUS "CMAKE_BUILD_TYPE:" ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_SYSTEM_NAME:" ${CMAKE_SYSTEM_NAME})
message(STATUS "XENABLE_TEST3:" ${XENABLE_TEST3})

find_package(Threads REQUIRED)

# include header dirs
include_directories(${CMAKE_SOURCE_DIR})   # project dir
include_directories(${CMAKE_SOURCE_DIR}/third-party/include)   # project dir
include_directories(${CMAKE_CURRENT_BINARY_DIR})  # current CMakeLists.txt dir (including sub dir)

# link lib dirs
link_directories(${CMAKE_SOURCE_DIR}/third-party/lib)
link_directories(${LIBRARY_OUTPUT_PATH})  # generate in building

#add_subdirectory(demo/bench)
add_subdirectory(demo/echo)
add_subdirectory(epoll)
add_subdirectory(mbase)
add_subdirectory(message_handle)
#add_subdirectory(transport)

