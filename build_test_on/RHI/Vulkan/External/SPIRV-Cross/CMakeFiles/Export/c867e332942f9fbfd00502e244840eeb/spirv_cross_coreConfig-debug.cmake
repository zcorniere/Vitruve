#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "spirv-cross-core" for configuration "Debug"
set_property(TARGET spirv-cross-core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(spirv-cross-core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libspirv-cross-core.a"
  )

list(APPEND _cmake_import_check_targets spirv-cross-core )
list(APPEND _cmake_import_check_files_for_spirv-cross-core "${_IMPORT_PREFIX}/lib/libspirv-cross-core.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
