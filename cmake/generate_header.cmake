function(generate_export_header macro_name output_path)
    # Compose the define name from macro_name
    string(REPLACE "_API" "_EXPORTS" macro_define "${macro_name}")

    # Use set_property to cache the values for configure_file
    set(EXPORT_MACRO "${macro_name}")
    set(EXPORT_DEFINE "${macro_define}")

    configure_file(${CMAKE_SOURCE_DIR}/cmake/EngineExport.hxx.in "${output_path}" @ONLY)

endfunction(generate_export_header)

macro(force_include_api_macro_header target header)
    if(MSVC)
        target_compile_options(${target} PRIVATE /FI"${header}")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -include "${header}")
    else()
        message(FATAL "Unknown compiler for force include: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endmacro(force_include_api_macro_header)
