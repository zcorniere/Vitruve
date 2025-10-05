project(VitruveRHIVulkan)
set(CMAKE_FOLDER "Vitruve/RHI/Vulkan")

add_subdirectory(External/)

option(VIT_ENABLE_VULKAN_DEBUGGING "" ON)

file(
    GLOB
    VULKAN_RENDERER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRHI/*.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRHI/Resources/*.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRHI/RenderPass/*.cxx
)

generate_export_header(VULKAN_RHI_API "${CMAKE_CURRENT_BINARY_DIR}/generated/VulkanRHI_Export.hxx")

add_library(
    ${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${VULKAN_RENDERER_FILES} # Compile the imgui vulkan backend #
                                            ${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/generated/VulkanRHI_Export.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/EngineExport.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/AssetRegistryExport.hxx" # TODO: try to make it automagically
    "${CMAKE_BINARY_DIR}/Engine/generated/BuildConfig.hxx"
    "VulkanRHI.pch.hxx"
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_definitions(${PROJECT_NAME} PRIVATE VULKAN_RHI_EXPORTS)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE imgui
            spirv-cross-core
            shaderc
            VulkanMemoryAllocator
            Vulkan::UtilityHeaders
            Vulkan::Headers
)
target_link_libraries(${PROJECT_NAME} PUBLIC VitruveEngine)
target_disable_rtti(${PROJECT_NAME})

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
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1)
if(VIT_ENABLE_VULKAN_DEBUGGING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_DEBUGGING_ENABLED=1)
endif()

if(OPTIMIZE_FOR_NATIVE)
    target_compile_options(${PROJECT_NAME} PUBLIC -march=native)
endif(OPTIMIZE_FOR_NATIVE)

if(VIT_ENABLE_PROFILING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif(VIT_ENABLE_PROFILING)

# ------------- Tests -------------

build_tests(${PROJECT_NAME} tests/ShaderCompiler.cxx) # Give the tests access to the RHI headers
target_include_directories(${PROJECT_NAME}_Test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}_Test PRIVATE Vulkan::Headers)
target_compile_definitions(${PROJECT_NAME}_Test PRIVATE VK_NO_PROTOTYPES=1)
target_precompile_headers(
    ${PROJECT_NAME}_Test
    PRIVATE
    "${CMAKE_BINARY_DIR}/Engine/generated/BuildConfig.hxx"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/VulkanRHI_Export.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/EngineExport.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/AssetRegistryExport.hxx" # TODO: try to make it automagically
    "VulkanRHI.pch.hxx"
)
