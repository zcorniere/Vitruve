project(RaphaelEditor)
set(CMAKE_FOLDER "Raphael/Editor")

add_executable(${PROJECT_NAME} EditorApplication.cxx Actor/Oscillator.cxx)

target_link_libraries(${PROJECT_NAME} PUBLIC RaphaelEngine imgui)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Engine/src/Engine/Raphael.hxx)
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
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 1)
target_disable_rtti(${PROJECT_NAME})

if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(RPH_ENABLE_PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_PROFILING=1)
    if(RPH_ENABLE_MEMORY_PROFILING)
        target_compile_definitions(${PROJECT_NAME} PUBLIC RPH_ENABLE_MEMORY_PROFILING=1)
    endif(RPH_ENABLE_MEMORY_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(RPH_ENABLE_PROFILING)

# target_generate_debug_file(${PROJECT_NAME})
