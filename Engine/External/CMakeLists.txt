set(CMAKE_FOLDER "Vitruve/Engine/External")

# logger
set(CPPLOGGER_NO_EXCEPTIONS
    ON
    CACHE BOOL "" FORCE
)
if(${VIT_LIBRARY_TYPE} STREQUAL "SHARED")
    set(CPPLOGGER_SHARED_LIB
        ON
        CACHE BOOL "" FORCE
    )
else()
    set(CPPLOGGER_SHARED_LIB
        OFF
        CACHE BOOL "" FORCE
    )
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp EXCLUDE_FROM_ALL)

# magic_enum
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum EXCLUDE_FROM_ALL)

# xdgpp
add_library(xdgpp INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(xdgpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xdgpp)
target_compile_features(xdgpp INTERFACE cxx_std_17)

# mimalloc
set(MI_OVERRIDE
    OFF
    CACHE BOOL "" FORCE
)
set(MI_BUILD_TESTS
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_OBJECT
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_SHARED
    OFF
    CACHE INTERNAL "" FORCE
)
if (NOT MSVC)
    set(MI_TRACK_VALGRIND "$<IF:$<AND:$<CONFIG:Debug>,$<NOT:$<BOOL:${VIT_ENABLE_ASAN}>>>,ON,OFF>")
endif()
set(MI_TRACK_ASAN ${VIT_ENABLE_ASAN})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mimalloc EXCLUDE_FROM_ALL)

# glfw
set(GLFW_BUILD_DOCS
    OFF
    CACHE INTERNAL "" FORCE
)
set(GLFW_INSTALL
    OFF
    CACHE INTERNAL "" FORCE
)
set(GLFW_LIBRARY_TYPE
    ${VIT_LIBRARY_TYPE}
    CACHE STRING "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glfw EXCLUDE_FROM_ALL)
if(VIT_SELECTED_RHI STREQUAL "Vulkan")
    target_link_libraries(glfw INTERFACE Vulkan::Headers)
    target_compile_definitions(glfw INTERFACE GLFW_INCLUDE_NONE)
endif()

# Modern Dialogs
add_library(
    ModernDialogs STATIC EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/ModernDialogs/ModernDialogs/ModernDialogs.cpp
)
if(NOT VIT_COMPILE_MONOLITHIC)
    set_property(TARGET ModernDialogs PROPERTY POSITION_INDEPENDENT_CODE ON)
endif(NOT VIT_COMPILE_MONOLITHIC)
target_include_directories(ModernDialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ModernDialogs/ModernDialogs)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm EXCLUDE_FROM_ALL)
