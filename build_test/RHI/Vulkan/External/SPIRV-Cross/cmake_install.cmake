# Install script for directory: /home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/spirv-cross-c.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/libspirv-cross-core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv_cross" TYPE FILE FILES
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/GLSL.std.450.h"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_common.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_cross_containers.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_cross_error_handling.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_cross.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_parser.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_cross_parsed_ir.hpp"
    "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_cfg.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake/spirv_cross_coreConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake/spirv_cross_coreConfig.cmake"
         "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/c867e332942f9fbfd00502e244840eeb/spirv_cross_coreConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake/spirv_cross_coreConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake/spirv_cross_coreConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/c867e332942f9fbfd00502e244840eeb/spirv_cross_coreConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spirv_cross_core/cmake" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/c867e332942f9fbfd00502e244840eeb/spirv_cross_coreConfig-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/libspirv-cross-glsl.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/spirv_cross" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/RHI/Vulkan/External/SPIRV-Cross/spirv_glsl.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake/spirv_cross_glslConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake/spirv_cross_glslConfig.cmake"
         "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/e2b605c21150436e2d28319122566e16/spirv_cross_glslConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake/spirv_cross_glslConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake/spirv_cross_glslConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/e2b605c21150436e2d28319122566e16/spirv_cross_glslConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/spirv_cross_glsl/cmake" TYPE FILE FILES "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/CMakeFiles/Export/e2b605c21150436e2d28319122566e16/spirv_cross_glslConfig-debug.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/runner/work/Vitruve/Vitruve/build_test/RHI/Vulkan/External/SPIRV-Cross/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
