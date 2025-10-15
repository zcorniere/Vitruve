file(REMOVE_RECURSE
  "libshaderc_combined.a"
  "libshaderc_combined.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/shaderc_combined.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
