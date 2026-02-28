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
set(MI_WIN_REDIRECT
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_SHARED
    ${VIT_LIBRARY_TYPE}
    CACHE INTERNAL "" FORCE
)
if(NOT MSVC)
    set(MI_TRACK_VALGRIND
        "$<IF:$<AND:$<NOT:$<BOOL:${VIT_ENABLE_ASAN}>>,$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>> ,ON,OFF>"
        CACHE INTERNAL "" FORCE
    )
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
target_compile_definitions(glfw INTERFACE GLFW_INCLUDE_NONE)

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

# slang
set(SLANG_VERSION "2026.3.1")

if(WIN32)
    set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-windows-x86_64.zip")
    set(SLANG_HASH "47faf59a4966dcf636b758f103941be7113e9d43c10ee6e8697de7663bcfc51f")
elseif(UNIX)
    set(SLANG_ARCHIVE "slang-${SLANG_VERSION}-linux-x86_64.tar.gz")
    set(SLANG_HASH "5a3618768f3472cc32f8ef59f5c1e3dedabada6d0d92d5b84508bd5eb10a5e01")
else()
    message(FATAL_ERROR "Platform not supported !")
endif()

fetchcontent_declare(
    slang
    URL "https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/${SLANG_ARCHIVE}"
    URL_HASH SHA256=${SLANG_HASH}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

fetchcontent_makeavailable(slang)
fetchcontent_getproperties(slang)

add_library(slang::slang SHARED IMPORTED GLOBAL)
set_target_properties(
    slang::slang PROPERTIES IMPORTED_LOCATION "${slang_SOURCE_DIR}/lib/libslang.so" INTERFACE_INCLUDE_DIRECTORIES
                                                                                    "${slang_SOURCE_DIR}/include"
)
