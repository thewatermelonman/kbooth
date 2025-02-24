cmake_minimum_required(VERSION 3.10)
project(libdither C)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/libdither)
set(SRC_FILES
    ${SRC_DIR}/libdither.c
    ${SRC_DIR}/ditherimage.c
    ${SRC_DIR}/random.c
    ${SRC_DIR}/gamma.c
    ${SRC_DIR}/hash.c
    ${SRC_DIR}/queue.c
    ${SRC_DIR}/dither_dbs.c
    ${SRC_DIR}/dither_dotdiff.c
    ${SRC_DIR}/dither_errordiff.c
    ${SRC_DIR}/dither_kallebach.c
    ${SRC_DIR}/dither_ordered.c
    ${SRC_DIR}/dither_riemersma.c
    ${SRC_DIR}/dither_threshold.c
    ${SRC_DIR}/dither_varerrdiff.c
    ${SRC_DIR}/dither_pattern.c
    ${SRC_DIR}/dither_dotlippens.c
    ${SRC_DIR}/dither_grid.c
)

add_library(libdither SHARED ${SRC_FILES})

# Platform-specific settings
if(WIN32)
    set_target_properties(libdither PROPERTIES SUFFIX ".dll")
    target_compile_definitions(libdither PRIVATE "LIB_VERSION=\"${LIB_VERSION}\"")
    target_compile_options(libdither PRIVATE -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wformat=2 -Wundef -fno-common -O2 -Os -Wpedantic -pedantic -Werror -Wno-sign-conversion -Wno-strict-prototypes)
elseif(APPLE)
    set_target_properties(libdither PROPERTIES SUFFIX ".dylib")
    target_compile_options(libdither PRIVATE -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wformat=2 -Wundef -fno-common -O2 -Os -Wpedantic -pedantic -Werror -Wno-sign-conversion -Wno-strict-prototypes)
else()
    set_target_properties(libdither PROPERTIES SUFFIX ".so")
    target_compile_options(libdither PRIVATE -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wformat=2 -Wundef -fno-common -O2 -Os -Wpedantic -pedantic -Werror -Wno-sign-conversion -Wno-strict-prototypes)
endif()

# Include directories
target_include_directories(libdither PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
