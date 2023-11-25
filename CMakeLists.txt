cmake_minimum_required(VERSION 3.27)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
  TheNewGentleman
  VERSION 0.6.3
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

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
    src/TngUtil.cpp
    src/TngInis.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  FILES ${headers} ${sources}
)

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(
  ${PROJECT_NAME}
  AUTHOR ModiLogist 
  SOURCES ${headers} ${sources}
)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
        WINVER=0x0601	# windows 7, minimum supported version by skyrim special edition
        _WIN32_WINNT=0x0601
        "$<$<BOOL:${SKSE_SUPPORT_XBYAK}>:SKSE_SUPPORT_XBYAK=1>"
        "$<$<BOOL:${ENABLE_SKYRIM_SE}>:ENABLE_SKYRIM_SE=1>"
        "$<$<BOOL:${ENABLE_SKYRIM_AE}>:ENABLE_SKYRIM_AE=1>"
        "$<$<BOOL:${ENABLE_SKYRIM_VR}>:ENABLE_SKYRIM_VR=1>"
)

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
target_include_directories(
  ${PROJECT_NAME}
    PRIVATE 
    ${SIMPLEINI_INCLUDE_DIRS}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:src>
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

target_precompile_headers(
  ${PROJECT_NAME}
  PRIVATE
    inc/PCH.h
    ${SIMPLEINI_INCLUDE_DIRS}/SimpleIni.h
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


file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/out/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/out/**/*.pdb")

set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}"
)
set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}"
)
