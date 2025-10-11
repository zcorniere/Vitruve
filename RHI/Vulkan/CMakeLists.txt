project(VitruveEngine_VulkanRHI)
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
    ${PROJECT_NAME} ${VIT_LIBRARY_TYPE} EXCLUDE_FROM_ALL ${VULKAN_RENDERER_FILES} # Compile the imgui vulkan backend #
                    ${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.cpp
)
vit_set_standard_options(${PROJECT_NAME})
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
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES=1)
if(VIT_ENABLE_VULKAN_DEBUGGING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_DEBUGGING_ENABLED=1)
endif()

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
