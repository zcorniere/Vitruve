cmake_minimum_required(VERSION 3.31)
project(Vitruve)

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(
        FATAL_ERROR
            "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles."
    )
endif()

option(BUILD_TESTING "Build the unit tests" ON)

include(CheckIPOSupported)
include(GNUInstallDirs)
include(cmake/tests.cmake)
include(cmake/find_linker.cmake)
include(cmake/CompileShaders.cmake)
include(cmake/StripDebugFile.cmake)
include(cmake/disable_rtti.cmake)
include(cmake/generate_header.cmake)

# Option to force colored output of the compiler
option(VIT_FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(VIT_FORCE_COLORED_OUTPUT)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

option(VIT_ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(VIT_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif(VIT_ENABLE_ASAN)

find_linker()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

check_ipo_supported(RESULT supported)
if(supported)
    message(STATUS "LTO enabled")
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported")
endif()

option(OPTIMIZE_FOR_NATIVE "Build with -march=native" OFF)
if(OPTIMIZE_FOR_NATIVE)
    message(STATUS "Building with -march=native")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(WARNING "Native optimization is not supported for MSVC")
        set(OPTIMIZE_FOR_NATIVE OFF)
    endif()
else()
    message(STATUS "Building without -march=native")
endif()

option(VIT_BUILD_DOCUMENTATION "Build the Doxygen documentation" OFF)
if(VIT_BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
        doxygen_add_docs(doc Engine/src/ RHI/Vulkan/src/ README.md)
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()

option(VIT_ENABLE_PROFILING "Enable the profiling" OFF)
option(VIT_ENABLE_MEMORY_PROFILING "Enable the memory profiling" OFF)
if(VIT_ENABLE_PROFILING)
    message(STATUS "VIT - Profiling is enabled")

    if(VIT_ENABLE_MEMORY_PROFILING)
        message(STATUS "VIT - Memory profiling is enabled")
    else()
        message(STATUS "VIT - Memory profiling is disabled")
    endif(VIT_ENABLE_MEMORY_PROFILING)
else()
    message(STATUS "VIT - Profiling is disabled")
endif(VIT_ENABLE_PROFILING)

option(VIT_COMPILE_MONOLITHIC "Compile the engine as a monolithic" OFF)
if(VIT_COMPILE_MONOLITHIC)
    set(VIT_LIBRARY_TYPE STATIC)
    message(STATUS "VIT - Compiling as monolithic")
else()
    set(VIT_LIBRARY_TYPE SHARED)
    message(STATUS "VIT - Compiling as modular")
endif(VIT_COMPILE_MONOLITHIC)

# Purge the generated files and regenerate them
file(REMOVE "${CMAKE_BINARY_DIR}/generated/*")
generate_build_config_header(
    OUTPUT_PATH
    "${CMAKE_BINARY_DIR}/generated/BuildConfig.h"
    VERSION
    "0.0.1"
    TEMPLATE
    "${CMAKE_SOURCE_DIR}/cmake/BuildConfig.h.in"
)

function(vit_set_standard_options target precompile_header)
    target_disable_rtti(${target})
    target_compile_features(${target} PUBLIC cxx_std_23)
    set_target_properties(
        ${target}
        PROPERTIES CXX_STANDARD 23
                   CXX_STANDARD_REQUIRED YES
                   CXX_EXTENSIONS NO
    )
    target_compile_definitions(${target} PUBLIC VITRUVE_BUILD_TYPE="$<CONFIG>")

    if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
        target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
    else()
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES BUILD_RPATH "$ORIGIN:$ORIGIN/lib"
                       INSTALL_RPATH "$ORIGIN:$ORIGIN/lib"
                       CXX_VISIBILITY_PRESET hidden
                       VISIBILITY_INLINES_HIDDEN 1
        )
    endif()

    target_compile_options(
        ${target}
        PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                -Wall
                -Wextra
                -Wno-missing-field-initializers>
                $<$<CXX_COMPILER_ID:MSVC>:
                /Zc:preprocessor
                /W4
                /wd4267 # conversion from 'size_t' to 'T', possible loss of data
                /wd4244 # conversion from 'T' to 'T', possible loss of data
                /wd4201 # unamed union
                /wd4324 # structure was padded due to alignment specifier
                /wd4702 # unreachable code
                /wd5030> # unknown attribute
    )
    if(OPTIMIZE_FOR_NATIVE)
        target_compile_options(${target} PUBLIC -march=native)
    endif(OPTIMIZE_FOR_NATIVE)
    if(VIT_ENABLE_PROFILING)
        target_link_libraries(${target} PUBLIC Tracy::TracyClient)
    endif(VIT_ENABLE_PROFILING)

    # #TOCHECK: does cmake actually grab the generated headers even if the line to generated them is later ?
    file(GLOB GENERATED_HEADERS "${CMAKE_BINARY_DIR}/generated/*.h")
    target_precompile_headers(${target} PRIVATE ${GENERATED_HEADERS} ${precompile_header})
endfunction(vit_set_standard_options)

function(vit_add_platforms_entrypoint target)
    if(WIN32)
        target_sources(${target} PRIVATE Entrypoints/Windows/Main.cxx)
        set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE TRUE)
    elseif(UNIX)
        target_sources(${target} PRIVATE Entrypoints/Linux/Main.cxx)
    else()
        message(FATAL_ERROR "Platform not supported !")
    endif()
endfunction(vit_add_platforms_entrypoint)

function(vit_copy_dlls_to_target_dir TARGET_NAME)
    # Copy all additional DLL/shared library targets passed after TARGET_NAME
    foreach(DLL_TARGET IN LISTS ARGN)
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${DLL_TARGET}> $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copying $<TARGET_FILE:${DLL_TARGET}> to $<TARGET_FILE_DIR:${TARGET_NAME}>"
        )
    endforeach()
endfunction(vit_copy_dlls_to_target_dir)

add_subdirectory(External/)
add_subdirectory(Engine/)
add_subdirectory(Editor/)
add_subdirectory(RHI/Vulkan/)
