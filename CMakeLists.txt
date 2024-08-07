cmake_minimum_required(VERSION 3.22)
project(GarCide)

find_package(TBB)
find_package(Doxygen)

set(USE_PAR TRUE CACHE BOOL "Enable parallelism.")
set(GENERATE_DOC TRUE CACHE BOOL "Generate documentation.")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose between release and debug.")

if (${USE_PAR})
    if (${TBB_FOUND})
        message("Found TBB!")
    else()
        message("Could not find TBB! Parallelism is disabled.")
    endif()
endif()

if (${GENERATE_DOC})
    if (${DOXYGEN_FOUND})
        doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}/inc
            ${PROJECT_SOURCE_DIR}/lib
            ${PROJECT_SOURCE_DIR}/src
            ALL
            COMMENT "Generate documentation."
        )
    else()
        message("Could not find Doxygen! Cannot generate documentation.")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib)

add_subdirectory(src)