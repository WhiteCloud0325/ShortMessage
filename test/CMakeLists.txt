cmake_minimum_required (VERSION 2.6)
project (AccessServiceTest)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_FLAGS "-pipe -ggdb -std=c++0x -W -Wall -Wno-unused-parameter -Wno-deprecated-declarations -Wno-reorder -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(CMAKE_BUILD_TYPE "Debug")
ADD_DEFINITIONS(-DLINUX -DPTHREAD)
message(INFO "CMAKE BUILD TYPE:" ${CMAKE_BUILD_TYPE})
set(PROJECT_DIR  ${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB_RECURSE COMMONS_SOURCES "src/commons/*.cpp")
file(GLOB_RECURSE MAIN_SOURCES "src/*.cpp")


set(SERVICE_SOURCES ${COMMONS_SOURCES} ${MAIN_SOURCES})
set(SERVICE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src")



set(LIBBOOST_MODULE "../lib/libboost")
set(LIBBOOST_MODULE_INCLUDE ${LIBBOOST_MODULE}/include)
file(GLOB_RECURSE LIBBOOST_MODULE_LIB "${LIBBOOST_MODULE}/lib/*.a")


include_directories(
    src/
    ${SERVICE_INCLUDE}
    ${LIBBOOST_MODULE_INCLUDE}
    )

add_executable(client-test ${SERVICE_SOURCES})

target_link_libraries(client-test
    ${LIBBOOST_MODULE_LIB}
    pthread
    rt
    dl
    )
