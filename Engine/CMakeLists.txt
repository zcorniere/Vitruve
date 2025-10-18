project(VitruveEngine)
set(CMAKE_FOLDER "Vitruve/Engine")

add_subdirectory(External/)

option(VIT_NAN_CHECKS "Enable NaN checks in the engine" ON)
if(VIT_NAN_CHECKS)
    message(STATUS "VIT - NaN checks enabled")
else()
    message(STATUS "VIT - NaN checks disabled")
endif(VIT_NAN_CHECKS)

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

generate_export_header(ENGINE)

add_library(
    ${PROJECT_NAME} ${VIT_LIBRARY_TYPE}
    src/Engine/Main.cxx
    src/Engine/Core/Log.cxx
    src/Engine/Core/UUID.cxx
    src/Engine/Core/RTTI/RObject.cxx
    src/Engine/Core/Engine.cxx
    src/Engine/Core/Application.cxx
    src/Engine/Core/Window.cxx
    src/Engine/RHI/RHI.cxx
    src/Engine/RHI/RHIDefinitions.cxx
    src/Engine/RHI/RHICommandList.cxx
    src/Engine/RHI/RHICommand.cxx
    src/Engine/RHI/RHIScene.cxx
    src/Engine/Core/Memory/Memory.cxx
    src/Engine/Core/Memory/MiMalloc.cxx
    src/Engine/Core/Memory/StdMalloc.cxx
    src/Engine/Modules/ModuleManager.cxx
    src/Engine/GameFramework/Actor.cxx
    src/Engine/GameFramework/World.cxx
    src/Engine/GameFramework/CameraActor.cxx
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
    src/AssetRegistry/AssetRegistry.cxx
    src/AssetRegistry/Asset.cxx
    src/AssetRegistry/MeshFactory.cxx
    ${PLATFORM_SOURCE_FILES}
    ${COMPILER_SOURCE_FILE}
)
vit_set_standard_options(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx")
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC cpplogger
           magic_enum
           glfw
           mimalloc-static
           ModernDialogs
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:VIT_POISON_ALLOCATION>)
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_EXPORTS)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC xdgpp)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp)
endif()


# ------------- Tests -------------

build_tests(
    ${PROJECT_NAME}
    tests/Containers/Array.cxx
    tests/Containers/ArrayView.cxx
    tests/Containers/Tuple.cxx
    tests/Containers/Map.cxx
    tests/Math/Quaternion.cxx
    tests/Math/Vector.cxx
    tests/Math/Math.cxx
    tests/Math/Matrix.cxx
    tests/Math/Transform.cxx
    tests/Math/ViewPoint.cxx
    tests/Core/RTTI/RTTI.cxx
    tests/Core/RTTI/RTTIParameter.cxx
    tests/CommandLine.cxx
)
target_precompile_headers(
    ${PROJECT_NAME}_Test
    PRIVATE
    "${CMAKE_BINARY_DIR}/generated/BuildConfig.h"
    "${CMAKE_BINARY_DIR}/generated/ENGINE_API_Export.h"
    "src/Engine/Vitruve.hxx"
)
target_link_libraries(${PROJECT_NAME}_Test PRIVATE glm)
