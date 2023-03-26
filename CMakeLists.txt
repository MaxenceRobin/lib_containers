cmake_minimum_required(VERSION 3.16.3)
project(lib_containers)

# Variables --------------------------------------------------------------------

set(TARGET_NAME containers)

set(SOURCES
        private/lib_arrays.c
        private/lib_buffers.c
        private/lib_lists.c
        private/lib_types.c
        private/lib_vectors.c
        private/lib_maps.c
        private/lib_iterators.c
        private/lib_container_algos.c
)

set(PUBLIC_HEADERS
        public
)

set(PRIVATE_HEADERS
        private)

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
