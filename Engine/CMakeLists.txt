project(VitruveEngine)
set(CMAKE_FOLDER "Vitruve/Engine")

set(VIT_SELECTED_RHI
    "Vulkan"
    CACHE STRING "Select the RHI to use"
)

add_library(RHI_Selector INTERFACE)
if(VIT_SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(RHI_Selector INTERFACE VitruveRHIVulkan)
    target_compile_definitions(RHI_Selector INTERFACE Vitruve_SELECTED_RHI_VULKAN=1)
else()
    message(FATAL_ERROR "${VIT_SELECTED_RHI} is not supported !")
endif()

add_subdirectory(External/)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES src/Engine/Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB PLATFORM_SOURCE_FILES src/Engine/Platforms/Linux/*.cxx)
else()
    message(FATAL_ERROR "Platform Not supported !")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/ClangCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/GNUCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_SOURCE_FILE src/Engine/Compilers/MSVCCompiler.cxx)
else()
    message(FATAL_ERROR "Compiler not supported !")
endif()

add_library(
    ${PROJECT_NAME} STATIC
    src/Engine/Main.cxx
    src/Engine/Core/Log.cxx
    src/Engine/Core/UUID.cxx
    src/Engine/Core/RTTI/RObject.cxx
    src/Engine/Core/Engine.cxx
    src/Engine/Core/Application.cxx
    src/Engine/Core/Window.cxx
    src/Engine/Core/RHI/RHI.cxx
    src/Engine/Core/RHI/RHIDefinitions.cxx
    src/Engine/Core/RHI/RHICommandList.cxx
    src/Engine/Core/RHI/RHICommand.cxx
    src/Engine/Core/RHI/RHIScene.cxx
    src/Engine/Core/Memory/Memory.cxx
    src/Engine/Core/Memory/MiMalloc.cxx
    src/Engine/Core/Memory/StdMalloc.cxx
    src/Engine/GameFramework/Actor.cxx
    src/Engine/GameFramework/World.cxx
    src/Engine/GameFramework/CameraActor.cxx
    src/Engine/AssetRegistry/AssetRegistry.cxx
    src/Engine/AssetRegistry/Asset.cxx
    src/Engine/AssetRegistry/MeshFactory.cxx
    src/Engine/Serialization/StreamWriter.cxx
    src/Engine/Serialization/StreamReader.cxx
    src/Engine/Serialization/FileStream.cxx
    src/Engine/Misc/DataLocation.cxx
    src/Engine/Misc/Timer.cxx
    src/Engine/Misc/Utils.cxx
    src/Engine/Misc/Assertions.cxx
    src/Engine/Misc/CommandLine.cxx
    src/Engine/Math/SIMD/Transform_float.cxx
    src/Engine/Math/SIMD/Transform_double.cxx
    src/Engine/Threading/Thread.cxx
    src/Engine/Threading/ThreadPool.cxx
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC cpplogger
           RHI_Selector
           magic_enum
           glfw
           mimalloc-static
           ModernDialogs
)

target_compile_options(
    ${PROJECT_NAME}
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
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/Engine/Vitruve.hxx)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:VIT_POISON_ALLOCATION>)
target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
target_disable_rtti(${PROJECT_NAME})

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC xdgpp)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp)
endif()

build_tests(
    ${PROJECT_NAME}
    tests/Containers/Array.cxx
    tests/Containers/ArrayView.cxx
    tests/Containers/Tuple.cxx
    tests/Containers/Map.cxx
    tests/Containers/Tree.cxx
    tests/Math/Quaternion.cxx
    tests/Math/Vector.cxx
    tests/Math/Math.cxx
    tests/Math/Shapes.cxx
    tests/Math/Matrix.cxx
    tests/Math/Transform.cxx
    tests/Math/ViewPoint.cxx
    tests/Core/RTTI/RTTI.cxx
    tests/Core/RTTI/RTTIParameter.cxx
    tests/CommandLine.cxx
)
target_link_libraries(${PROJECT_NAME}_Test PRIVATE glm)

if(VIT_ENABLE_PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_ENABLE_PROFILING=1)
    if(VIT_ENABLE_MEMORY_PROFILING)
        target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_ENABLE_MEMORY_PROFILING=1)
    endif(VIT_ENABLE_MEMORY_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(VIT_ENABLE_PROFILING)

if(OPTIMIZE_FOR_NATIVE)
    target_compile_options(${PROJECT_NAME} PUBLIC -march=native)
endif()

option(VIT_NAN_CHECKS "Enable NaN checks in the engine" ON)
if(VIT_NAN_CHECKS)
    message(STATUS "VIT - NaN checks enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_NAN_CHECKS=1)
else()
    message(STATUS "VIT - NaN checks disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_NAN_CHECKS=0)
endif(VIT_NAN_CHECKS)
