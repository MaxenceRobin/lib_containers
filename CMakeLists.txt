cmake_minimum_required(VERSION 3.16.3)
project(libcontainers)

# Variables --------------------------------------------------------------------

set(TARGET_NAME containers)

set(SOURCES
        src/libcontainers.c
        src/libiterators.c
        src/libvectors.c
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