cmake_minimum_required(VERSION 3.5)
enable_language(C)

set(PROG_NAME "jnxutil")
add_definitions(-DPROG_NAME="${PROG_NAME}")
add_definitions(-DPROG_VERSION="0.0.1")

project(${PROG_NAME})
file(GLOB SOURCES "*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -O2 -fno-ident -pipe")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")

set(WITHOUT_GDAL on)
unset(WITHOUT_GDAL)

if (NOT DEFINED WITHOUT_GDAL)
    find_package(GDAL REQUIRED)
    include_directories(${GDAL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${GDAL_LIBRARY})
    add_definitions(-DWITH_GDAL)
else ()
    message(WARNING "!!! ${PROG_NAME} will be build without tiles export feature !!!")
endif ()

install (TARGETS ${PROG_NAME} DESTINATION bin)
install (FILES README.md LICENSE DESTINATION share/${PROG_NAME})
