cmake_minimum_required(VERSION 3.16)
project(byte_pair_encoding)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)


set(BPE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/include/bpe.h
  )

add_library(bpe)
target_sources(bpe
 PRIVATE
  ${BPE_SOURCES}
  )

target_include_directories(bpe
 PRIVATE
 ${INCLUDE}
)

set(TXT2BPE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/txt2bpe.c
)

add_executable(txt2bpe)
target_sources(txt2bpe
 PRIVATE
  ${TXT2BPE_SRC}
)

target_link_libraries(txt2bpe
  bpe
)

set(BPE2DOT_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpe2dot.c
)

add_executable(bpe2dot)
target_sources(bpe2dot
 PRIVATE
  ${BPE2DOT_SRC}
)

target_link_libraries(bpe2dot
  bpe
)

set(BPE_INSPECT_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bpe_inspect.c
)

add_executable(bpe_inspect)
target_sources(bpe_inspect
 PRIVATE
  ${BPE_INSPECT_SRC}
)

target_link_libraries(bpe_inspect
  bpe
)


