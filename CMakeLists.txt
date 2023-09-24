cmake_minimum_required(VERSION 3.27)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
  TheNewGenital
  VERSION 1.0.0
  LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

include(GNUInstallDirs)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
  @ONLY
)

set(PUBLIC_HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc/${PROJECT_NAME}")

set(
  headers
)

set(
  sources
    src/Main.cpp
    src/Hooks.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  FILES ${headers} ${sources}
)

find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(
	${PROJECT_NAME} 
	AUTHOR ModiLogist 
	SOURCES ${headers} ${sources}
)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:src>
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
	inc/PCH.h
)

install(
    DIRECTORY "${PUBLIC_HEADER_DIR}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_NAME "Debug")
else()
  set(BUILD_NAME "Release")
endif()

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/out/${BUILD_NAME}/skse/plugins"
)
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out/${BUILD_NAME}/skse/plugins"
)
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out/${BUILD_NAME}/skse/plugins"
)

set(OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/out/${BUILD_NAME}/skse/plugins/TheNewGenital.dll")
set(OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/out/${BUILD_NAME}/skse/plugins/TheNewGenital.pdb")

set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}"
)
set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}"
)
