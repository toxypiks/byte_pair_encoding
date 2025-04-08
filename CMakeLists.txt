cmake_minimum_required(VERSION 3.16)
project(byte_pair_encoding)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(MAIN_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  )

add_executable(main)
target_sources(main
 PRIVATE
  ${MAIN_SRC}
  )
