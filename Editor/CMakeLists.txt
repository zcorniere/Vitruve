project(VitruveEditor)
set(CMAKE_FOLDER "Vitruve")

add_executable(${PROJECT_NAME} EditorApplication.cxx Actor/Oscillator.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine imgui)
if(VIT_COMPILE_MONOLITHIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine_VulkanRHI)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE VitruveEngine_AssetRegistry)
    add_dependencies(${PROJECT_NAME} VitruveEngine_VulkanRHI)
endif()

vit_set_standard_options(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/Engine/src/Engine/Vitruve.hxx")
if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif(MSVC)

# target_generate_debug_file(${PROJECT_NAME})
