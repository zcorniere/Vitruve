project(VitruveEditor)
set(CMAKE_FOLDER "Vitruve")

add_executable(${PROJECT_NAME} EditorApplication.cxx Actor/Oscillator.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine VitruveEngine_AssetRegistry imgui)
if(VIT_COMPILE_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine_VulkanRHI)
else()
    add_dependencies(${PROJECT_NAME} VitruveEngine_VulkanRHI)
endif()

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
    "${CMAKE_BINARY_DIR}/Engine/generated/BuildConfig.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/EngineExport.hxx"
    "${CMAKE_BINARY_DIR}/Engine/generated/AssetRegistryExport.hxx" # TODO: try to make it automagically
    "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx"
)
vit_set_standard_options(${PROJECT_NAME})
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif(MSVC)

# target_generate_debug_file(${PROJECT_NAME})
