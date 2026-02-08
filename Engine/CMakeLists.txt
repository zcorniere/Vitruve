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
    file(GLOB VIT_PLATFORM_SOURCE_FILES src/Engine/Platforms/Windows/*.cxx)
elseif(UNIX)
    file(GLOB VIT_PLATFORM_SOURCE_FILES src/Engine/Platforms/Linux/*.cxx)
else()
    message(FATAL_ERROR "Platform Not supported !")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(VIT_COMPILER_SOURCE_FILE src/Engine/Compilers/ClangCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(VIT_COMPILER_SOURCE_FILE src/Engine/Compilers/GNUCompiler.cxx)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(VIT_COMPILER_SOURCE_FILE src/Engine/Compilers/MSVCCompiler.cxx)
else()
    message(FATAL_ERROR "Compiler not supported !")
endif()

generate_export_header(ENGINE)

set(VIT_ASSET_REGISTRY_SOURCES src/AssetRegistry/AssetRegistry.cxx src/AssetRegistry/Asset.cxx
                               src/AssetRegistry/MeshFactory.cxx
)

set(VIT_GAMEFRAMEWORK_SOURCES src/GameFramework/Actor.cxx src/GameFramework/World.cxx src/GameFramework/CameraActor.cxx
                              src/GameFramework/LightActor.cxx
)

set(VIT_RHI_SOURCES
    src/RHI/RHI.cxx
    src/RHI/RHIDefinitions.cxx
    src/RHI/RHICommandList.cxx
    src/RHI/RHICommand.cxx
    src/RHI/RHIScene.cxx
)

add_library(
    ${PROJECT_NAME}
    ${VIT_LIBRARY_TYPE}
    src/Engine/Main.cxx
    src/Engine/Core/Log.cxx
    src/Engine/Core/UUID.cxx
    src/Engine/Core/RTTI/RObject.cxx
    src/Engine/Core/Engine.cxx
    src/Engine/Core/Application.cxx
    src/Engine/Core/Window.cxx
    src/Engine/Core/Memory/Memory.cxx
    src/Engine/Core/Memory/MiMalloc.cxx
    src/Engine/Core/Memory/StdMalloc.cxx
    src/Engine/Modules/ModuleManager.cxx
    src/Engine/Serialization/StreamWriter.cxx
    src/Engine/Serialization/StreamReader.cxx
    src/Engine/Serialization/FileStream.cxx
    src/Engine/Misc/DataLocation.cxx
    src/Engine/Misc/Timer.cxx
    src/Engine/Misc/Utils.cxx
    src/Engine/Misc/Assertions.cxx
    src/Engine/Misc/CommandLine.cxx
    src/Engine/Misc/ConsoleVariable.cxx
    src/Engine/Math/SIMD/Transform_float.cxx
    src/Engine/Math/SIMD/Transform_double.cxx
    src/Engine/Math/Transform.cxx
    src/Engine/Threading/Thread.cxx
    src/Engine/Threading/ThreadPool.cxx
    ${VIT_PLATFORM_SOURCE_FILES}
    ${VIT_COMPILER_SOURCE_FILE}
    ${VIT_ASSET_REGISTRY_SOURCES}
    ${VIT_RHI_SOURCES}
    ${VIT_GAMEFRAMEWORK_SOURCES}
)
vit_set_standard_options(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx")
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC cpplogger
           magic_enum
           glfw
           mimalloc-static
           imgui
           ModernDialogs
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:VIT_POISON_ALLOCATION>)
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_EXPORTS)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC xdgpp)

    if(VIT_ENABLE_STACKTRACE)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBDW REQUIRED libdw)
        target_include_directories(${PROJECT_NAME} PRIVATE ${LIBDW_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBDW_LIBRARIES})
        target_compile_options(${PROJECT_NAME} PRIVATE ${LIBDW_CFLAGS_OTHER})
    endif(VIT_ENABLE_STACKTRACE)
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
