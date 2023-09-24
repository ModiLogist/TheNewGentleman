cmake_minimum_required(VERSION 3.22)

project(TheNewGenital VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(
  headers
	inc/PCH.h
    )

set(
  sources
    src/Main.cpp
)

find_package(CommonLibSSE CONFIG REQUIRED)

add_commonlibsse_plugin(
	${PROJECT_NAME} 
	AUTHOR ModiLogist 
	SOURCES ${headers} ${sources}
	)

add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		inc/PCH.h
	)

file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/out/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/out/**/*.pdb")

