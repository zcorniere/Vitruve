project(VitruveEditor)
set(CMAKE_FOLDER "Vitruve")

add_executable(${PROJECT_NAME} src/EditorApplication.cxx src/Actor/Oscillator.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
if(VIT_COMPILE_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine_VulkanRHI)
else()
    add_dependencies(${PROJECT_NAME} VitruveEngine_VulkanRHI)
endif()

vit_add_platforms_entrypoint(${PROJECT_NAME})
vit_set_standard_options(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx")
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)

vit_copy_dlls_to_target_dir(${PROJECT_NAME} VitruveEngine VitruveEngine_VulkanRHI glfw cpplogger)

# target_generate_debug_file(${PROJECT_NAME})
