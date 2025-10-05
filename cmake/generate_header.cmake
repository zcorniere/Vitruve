function(generate_export_header macro_name output_path)
    # Compose the define name from macro_name
    string(REPLACE "_API" "_EXPORTS" macro_define "${macro_name}")

    # Use set_property to cache the values for configure_file
    set(EXPORT_MACRO "${macro_name}")
    set(EXPORT_DEFINE "${macro_define}")

    configure_file(${CMAKE_SOURCE_DIR}/cmake/EngineExport.hxx.in "${output_path}" @ONLY)

endfunction()
