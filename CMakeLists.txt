# Project
cmake_minimum_required(VERSION 3.23)
project(spring-it-on)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Sources
file(GLOB SOURCES *.c)
list(FILTER SOURCES EXCLUDE REGEX ".+/common.c$")
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CXX)

# Targets
foreach (SOURCE ${SOURCES})
    get_filename_component(BASE_FILENAME ${SOURCE} NAME_WLE)
    set(TARGET ${BASE_FILENAME})
    add_executable(${TARGET} ${SOURCE} )
    target_link_libraries(${TARGET} raylib)
endforeach(SOURCE)
