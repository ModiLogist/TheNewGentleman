cmake_minimum_required(VERSION 3.22)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

project(TheNewGenital VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

set(
	headers
	inc/Hooks.h
)

set(
	sources
	src/Hooks.cpp
	src/Main.cpp
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

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
	inc/PCH.h
)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/out${BUILD_NAME}/skse/plugins"
)
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out${BUILD_NAME}/skse/plugins"
)
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out${BUILD_NAME}/skse/plugins"
)

set(OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/out${BUILD_NAME}/skse/plugins/TheNewGenital.dll")
set(OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/out${BUILD_NAME}/skse/plugins/TheNewGenital.pdb")

set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}"
)
set_property(
  TARGET ${PROJECT_NAME}
  APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}"
)
