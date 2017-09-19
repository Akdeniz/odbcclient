cmake_minimum_required(VERSION 3.3)
project(odbcclient)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options filesystem system REQUIRED )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} "nanodbc" ${Boost_INCLUDE_DIR} )

find_path(UnixODBC_INCLUDE_DIR uodbc_stats.h
  /usr/include
  /usr/local/include
  /usr/include/odbc
  /usr/local/include/odbc
  /usr/include/libodbc
  /usr/local/include/libodbc)
if(UnixODBC_INCLUDE_DIR)
  set(ODBC_CONFIG 1)
  message(STATUS "nanodbc build: ODBC on Unix - unixODBC")
  set(ODBCLIB odbc)
  set(ODBC_CFLAGS "-I${UnixODBC_INCLUDE_DIR} -DHAVE_UNISTD_H -DHAVE_PWD_H -DHAVE_SYS_TYPES_H -DHAVE_LONG_LONG -DSIZEOF_LONG_INT=8")
endif()

set(SOURCE_FILES main.cpp nanodbc/nanodbc.h nanodbc/nanodbc.cpp)
add_executable(odbcclient ${SOURCE_FILES})

set_target_properties(odbcclient PROPERTIES
COMPILE_FLAGS "${ODBC_CFLAGS}"
LIBRARY_OUTPUT_DIRECTORY "lib")

TARGET_LINK_LIBRARIES(odbcclient LINK_PUBLIC ${ODBCLIB} ${Boost_LIBRARIES})