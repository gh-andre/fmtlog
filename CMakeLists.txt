#
# This is a test project and various options haven't been tested
# thoroughly. See README.md for more information.
#

cmake_minimum_required(VERSION 3.15)

project(fmtlog CXX)

set(CMAKE_CXX_STANDARD 20)

set(FMTLOG_LIB_NAME fmtlog)

option(FMTLOG_BUILD_TEST "Build fmtlog tests?" OFF)
option(FMTLOG_BUNDLED_FMT "Bundle fmt with fmtlog?" OFF)
option(FMTLOG_FULL_BUFFER_BLOCK "Block when buffer is full (ON) or discard (OFF)?" OFF)

#
# fmtlog source doesn't implement exports with __declspec(dllexport)
# and while a DLL will be built, it will not have any visible symbols
# and there will be no import library generated.
#
if(MSVC AND BUILD_SHARED_LIBS)
  message(FATAL_ERROR "Building fmtlog DLL on Windows is not implemented")
endif()

#
# If a static library may be consumed by a shared library, it must
# be generated with the -fPIC option. For the purposes of this project
# no alternative is provided, but for standalone executables built
# without -fPIC, this setting can be made conditional.
#
if(NOT BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

#
# Multi-configuration generators ignore CMAKE_BUILD_TYPE and should
# be configured via generator expressions (e.g. $<CONFIG:DEBUG>) and
# will be activated when --config command line option is used.
#
# Single-configuration generators require CMAKE_BUILD_TYPE assigned
# some configuration name. CMake defines this variable as an empty
# string.
#
if(NOT GENERATOR_IS_MULTI_CONFIG)
  # build release configuration by default
  if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

if(MSVC)
    #
    # VC++ uses the default character encoding, which is typically
    # Win-1252 for Western Windows configurations. fmt by default
    # requires /utf-8 option to set the default source and execution
    # character sets to UTF-8, which would throw off many existing
    # projects.
    #
    # If you are building a newer project and do want UTF-8 as the
    # default character set, change remove FMT_UNICODE=OFF in
    # get-fmt.bat and replace this line with the one shown below.
    #
    #   add_compile_options(/utf-8)
    #
    add_compile_definitions(FMT_UNICODE=0)
else()
    # set up install directories for this build (e.g. lib vs. lib64)
    include(GNUInstallDirs)

    # CMAKE_CXX_FLAGS_RELEASE already has -O3 and CMAKE_CXX_STANDARD sets -std=c++20
    add_compile_options(-Wall)
    #add_compile_options(-Wall -Ofast -std=c++20 -DNDEBUG -march=skylake -flto -fno-exceptions -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables -DFMTLOG_NO_CHECK_LEVEL=1)
    #SET(CMAKE_AR  "gcc-ar")
    #SET(CMAKE_RANLIB  "gcc-ranlib")
    link_libraries(pthread)
endif()

#
# If fmt is not available as a package, use get-fmt.bat or
# get-fmt.sh to download and build fmt as a standalone library.
#
# If fmt is not installed into a standard location, set these
# variables to point to fmt's include and library directories.
#
if(FMTLOG_BUNDLED_FMT)
    set(FMTLOG_FMT_INCLUDE_DIR fmt/include)

    # CMake adds configuration, as if `fmt/$<CONFIG>` was also specified (e.g. ...\build\fmt\Release\fmt.lib)
    set(FMTLOG_FMT_LIB_DIR ${CMAKE_BINARY_DIR}/fmt)
else()
    if(NOT DEFINED FMTLOG_FMT_INCLUDE_DIR)
        # may be in /usr/local/include (local build) or /usr/include (package manager)
        if(MSVC)
            message(FATAL_ERROR "FMTLOG_FMT_INCLUDE_DIR must point to fmt's include directory")
        endif()
    endif()

    if(NOT DEFINED FMTLOG_FMT_LIB_DIR)
        if(MSVC)
            message(FATAL_ERROR "FMTLOG_FMT_LIB_DIR must point to fmt's library directory")
        endif()
    endif()
endif()

#
# fmtlog source has been modified to include fmt headers using
# the angle bracket syntax, so we need to add the include path
# in order for this configuration to build.
#
include_directories(${FMTLOG_FMT_INCLUDE_DIR})

#
# This configuration expects fmt in the root directory of this
# project and may be obtained either by initializing the Git
# submodule or just by extracting the expected version of fmt
# under the fmt directory.
#
if(FMTLOG_BUNDLED_FMT)
  add_subdirectory(fmt)

  #
  # Current version of fmt requires this directive to be able to
  # install header files and fails otherwise with not being able
  # to locate agrs.h and other headers.
  #
  set_target_properties(fmt PROPERTIES PUBLIC_HEADER "")
  
  #
  # fmt detects when it's included as a subdirectory and avoids
  # installing header files, so they have to be installed
  # explicitly. Install fmt headers under fmtlog to avoid conflicts
  # with a standalone fmt installation.
  #
  install(DIRECTORY ${FMTLOG_FMT_INCLUDE_DIR} DESTINATION ".")

  #
  # Place fmt libraries under fmtlog to avoid conflicts with a
  # standalone fmt installation.
  #
  install(TARGETS fmt DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(FILES fmtlog.h fmtlog-inl.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fmtlog)

if(BUILD_SHARED_LIBS)
  add_library(${FMTLOG_LIB_NAME} SHARED fmtlog.cc)

  if(DEFINED FMTLOG_FMT_LIB_DIR)
    target_link_directories(${FMTLOG_LIB_NAME} PUBLIC ${FMTLOG_FMT_LIB_DIR})
  endif()

  target_link_libraries(${FMTLOG_LIB_NAME} PUBLIC fmt$<$<CONFIG:Debug>:d>)
  install(TARGETS ${FMTLOG_LIB_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
else ()
  add_library(${FMTLOG_LIB_NAME} STATIC fmtlog.cc)
  install(TARGETS ${FMTLOG_LIB_NAME} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

set_target_properties(${FMTLOG_LIB_NAME} PROPERTIES DEBUG_POSTFIX d)

# must also be defined in projects using fmtlog (used in an inline function)
target_compile_definitions(${FMTLOG_LIB_NAME} PUBLIC -DFMTLOG_BLOCK=$<BOOL:${FMTLOG_FULL_BUFFER_BLOCK}>)

if(FMTLOG_BUNDLED_FMT)
  add_dependencies(${FMTLOG_LIB_NAME} fmt)
endif()

if(FMTLOG_BUILD_TEST)
  add_subdirectory(test)
endif()
