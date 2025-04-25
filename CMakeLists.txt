cmake_minimum_required(VERSION 3.20)

# ---- Project ----
set(NAME "TheNewGentleman" CACHE STRING "")
set(VR_VERSION 1)
project(
    ${NAME}
    VERSION 4.1.0
    LANGUAGES CXX
)

# ---- Options ----
option(BUILD_SKYRIMVR "Build for Skyrim VR" OFF)

# ---- Cache build vars ----

macro(set_from_environment VARIABLE)
    if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
        set(${VARIABLE} $ENV{${VARIABLE}})
    endif()
endmacro()

macro(find_commonlib_path)
    if(CommonLibName AND NOT ${CommonLibName} STREQUAL "")
        # Check ext
        find_path(CommonLibPath
            include/REL/Relocation.h
            PATHS ext/${CommonLibName})
        if(${CommonLibPath} STREQUAL "CommonLibPath-NOTFOUND")
            #Check path
            set_from_environment(${CommonLibName}Path)
            set(CommonLibPath ${${CommonLibName}Path})
        endif()
    endif()
endmacro()

set_from_environment(VCPKG_ROOT)
if(BUILD_SKYRIMVR)
    add_compile_definitions(SKYRIMVR)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CommonLibName "CommonLibVR")
    set_from_environment(SkyrimVRPath)
    set(SkyrimVersion "Skyrim VR")
    set(VERSION ${VERSION}.${VR_VERSION})
else()
    add_compile_definitions(SKYRIMFLATRIM)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CommonLibName "CommonLibSSE")
    set_from_environment(Skyrim64Path)
    set(SkyrimVersion "Skyrim universal")
endif()
find_commonlib_path()

if(DEFINED VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else()
    message(
        WARNING
        "Variable VCPKG_ROOT is not set. Continuing without vcpkg."
    )
endif()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")



configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/inc/Version.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
        "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Globals ----
if(MSVC)
    if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(
            /MP # Build with Multiple Processes
        )
    endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)

# ---- Dependencies ----
if(DEFINED CommonLibPath AND NOT ${CommonLibPath} STREQUAL "" AND IS_DIRECTORY ${CommonLibPath})
    add_subdirectory(${CommonLibPath} ${CommonLibName})
else()
    message(
        FATAL_ERROR
        "Variable ${CommonLibName}Path is not set."
    )
endif()

# ---- Add source files ----

include(cmake/headerlist.cmake)
include(cmake/sourcelist.cmake)

source_group(
    TREE
    ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
    ${headers}
    ${sources}
)

source_group(
    TREE
    ${CMAKE_CURRENT_BINARY_DIR}
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/inc/Version.h
)

# ---- Create DLL ----

add_library(
    ${PROJECT_NAME}
    SHARED
    ${headers}
    ${sources}
    ${CMAKE_CURRENT_BINARY_DIR}/inc/Version.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    .clang-format)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
    cxx_std_23
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    _UNICODE
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${SIMPLEINI_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${CommonLibName}::${CommonLibName}
)

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
    inc/PCH.h
    ${SIMPLEINI_INCLUDE_DIRS}/SimpleIni.h
)

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        /sdl # Enable Additional Security Checks
        /utf-8 # Set Source and Executable character sets to UTF-8
        /Zi # Debug Information Format

        /permissive- # Standards conformance
        /Zc:preprocessor # Enable preprocessor conformance mode
        /W4 # Warning Level 4
        /wd4200 # nonstandard extension used : zero-sized array in struct/union
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
        "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
        "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif()

# ---- Post build ----
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/out/${SkyrimVersion}/skse/plugins"
)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out/${SkyrimVersion}/skse/plugins"
)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/out/${SkyrimVersion}/skse/plugins"
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
