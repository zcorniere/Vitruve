project(VitruveEditor)
set(CMAKE_FOLDER "Vitruve")

add_executable(${PROJECT_NAME} EditorApplication.cxx Actor/Oscillator.cxx)
add_dependencies(${PROJECT_NAME} VitruveRHIVulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine VitruveEngine_AssetRegistry imgui)
target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
    "${CMAKE_BINARY_DIR}/Engine/generated/EngineExport.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/AssetRegistryExport.hxx" # TODO: try to make it automagically
    "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx"
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
            /wd5030 # unknown attribute
            >
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_disable_rtti(${PROJECT_NAME})

if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(VIT_ENABLE_PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_ENABLE_PROFILING=1)
    if(VIT_ENABLE_MEMORY_PROFILING)
        target_compile_definitions(${PROJECT_NAME} PUBLIC VIT_ENABLE_MEMORY_PROFILING=1)
    endif(VIT_ENABLE_MEMORY_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(VIT_ENABLE_PROFILING)

# target_generate_debug_file(${PROJECT_NAME})
