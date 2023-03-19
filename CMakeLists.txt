cmake_minimum_required(VERSION 3.16.3)
project(lib_containers)

# Variables --------------------------------------------------------------------

set(TARGET_NAME containers)

set(SOURCES
        src/lib_arrays.c
        src/lib_buffers.c
        src/lib_lists.c
        src/lib_types.c
        src/lib_vectors.c
        src/lib_maps.c
        src/lib_iterators.c
        src/lib_container_algos.c
)

set(PUBLIC_HEADERS
        include/public
)

set(PRIVATE_HEADERS
        include/private)

set(INSTALL_DIR
        bin
)

# Configuration ----------------------------------------------------------------

add_library(${TARGET_NAME} SHARED ${SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${PUBLIC_HEADERS})
target_include_directories(${TARGET_NAME} PRIVATE ${PRIVATE_HEADERS})

set_target_properties(${TARGET_NAME}
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${INSTALL_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${INSTALL_DIR}
        C_STANDARD 11
)
