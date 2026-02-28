set(CMAKE_FOLDER "Vitruve/RHI/Vulkan/External")

# Vulkan Header
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers EXCLUDE_FROM_ALL)

# Vulkan Helpers
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Utility-Libraries EXCLUDE_FROM_ALL)

# vma
set(VMA_STATIC_VULKAN_FUNCTIONS
    OFF
    CACHE BOOL "" FORCE
)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS
    OFF
    CACHE BOOL "" FORCE
)
set(VMA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator)
add_library(VulkanMemoryAllocator STATIC EXCLUDE_FROM_ALL ${VMA_PATH}/include/vk_mem_alloc.h)
set_source_files_properties(${VMA_PATH}/include/vk_mem_alloc.h PROPERTIES LANGUAGE CXX)
set_property(TARGET VulkanMemoryAllocator PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(
    VulkanMemoryAllocator
    PRIVATE VMA_IMPLEMENTATION
    PUBLIC VMA_STATIC_VULKAN_FUNCTIONS=0
    PUBLIC VMA_DYNAMIC_VULKAN_FUNCTIONS=1
)
target_include_directories(VulkanMemoryAllocator PUBLIC ${VMA_PATH}/include)
set_target_properties(VulkanMemoryAllocator PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Headers)
