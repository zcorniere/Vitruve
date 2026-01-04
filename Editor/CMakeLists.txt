project(VitruveEditor)
set(CMAKE_FOLDER "Vitruve")

add_executable(${PROJECT_NAME} src/EditorApplication.cxx src/Components/Oscillator.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
vit_link_to_module(${PROJECT_NAME} VitruveEngine_VulkanRHI "RHIVulkan")
vit_copy_dlls_to_target_dir(${PROJECT_NAME} VitruveEngine)

vit_add_platforms_entrypoint(${PROJECT_NAME})
vit_set_standard_options(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx")
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)

# target_generate_debug_file(${PROJECT_NAME})
