cmake_minimum_required(VERSION 3.15)

#
# This is a test project and various options haven't been tested
# thoroughly. Use at your own risk.
#
# Building a static library on single-configuration systems (e.g.
# Makefiles):
#
#     cmake -S path-to-fmtlog-source \
#           -B path-to-build-dir/Release \
#           -DCMAKE_BUILD_TYPE=Release
#
#     cmake --build path-to-build-dir/Release
#
# Building a shared library on single-configuration systems (e.g.
# Makefiles):
#
#     cmake -S path-to-fmtlog-source \
#           -B path-to-build-dir/Release \
#           -DCMAKE_BUILD_TYPE=Release \
#           -DBUILD_SHARED_LIBS=ON
#
#     cmake --build path-to-build-dir/Release
#
# Building a static library on multi-configuration systems (e.g.
# Visual Studio):
#
#     cmake -S path-to-fmtlog-source \
#           -B path-to-build-dir/Release
#
#     cmake --build path-to-build-dir/Release --config Release
#
# Building a shared library on multi-configuration systems (e.g.
# Visual Studio) is not implemented because fmtlog source has
# no explicit symbol export declarations.
#
# All configuration commands above assume fmt is installed into
# a globally-configured location, such as /usr/include and /usr/lib.
# In order to build against a fmt bundled with fmtlog, which can
# be either a Git sub-module or just a copy of fmt of the right
# version in the fmt directory, add this option to the step that
# generates build files:
#
#     -DFMTLOG_BUNDLED_FMT=ON
#
# Test utilities may be built using this option in the step that
# generates build files:
#
#     -DFMTLOG_BUILD_TEST=ON
#
# Benchmark configurations haven't been adapted for this project
# and will not work. Benchmark sub-directory is included at the
# end of the file just to keep it visible.
#

project(fmtlog CXX)

set(FMTLOG_LIB_NAME fmtlog)

option(FMTLOG_BUILD_TEST "Build fmtlog tests?" OFF)
option(FMTLOG_BUNDLED_FMT "Bundle fmt with fmtlog?" OFF)

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
# Single-configuration generators require CMAKE_BUILD_TYPE defined
# to work. For example, DEBUG_POSTFIX may be ignored when generating
# a Makefile and CMAKE_BUILD_TYPE is not defined.
#
if(NOT GENERATOR_IS_MULTI_CONFIG)
  # build release configuration by default
  if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

if(NOT MSVC)
  # set up install directories for this build (e.g. lib vs. lib64)
  include(GNUInstallDirs)
endif()

if(MSVC)
  set(FMTLOG_BUILD_BENCH OFF)
else()
  option(FMTLOG_BUILD_BENCH "[NOT MAINTAINED] Build benchmark tests?" OFF)
endif()

if(MSVC)
  add_compile_options(/std:c++latest)
else()
  add_compile_options(-Wall -std=c++17)
    #add_compile_options(-Wall -Ofast -std=c++2a -flto -fno-exceptions -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables -DFMTLOG_NO_CHECK_LEVEL=1)
    #SET(CMAKE_AR  "gcc-ar")
    #SET(CMAKE_RANLIB  "gcc-ranlib")
  link_libraries(pthread)
endif()

#
# This configuration expects fmt in the root directory of this
# project and may be obtained either by initializing the Git
# submodule or just by extracting the expected version of fmt
# under the fmt directory.
#
# NOTE: This configuration is not complete and will generate
# inconsistent installation with regards to how fmtlog uses
# fmt, as described inside this conditional. Standalone fmt
# installation should be used when possible.
#
if(FMTLOG_BUNDLED_FMT)
  add_subdirectory(fmt)

  #
  # fmtlog source has been modified to include fmt headers using
  # the angle bracket syntax, so we need to add the include path
  # in order for this configuration to build.
  #
  # NOTE: While this works for this build, it will install fmtlog
  # headers that use angle bracket includes, which means standalone
  # fmt headers may be picked up if a different version of fmt
  # is installed. In order for this configuration to work properly,
  # bundled fmt headers must be included in fmtlog source via
  # quoted paths, which would require fmtlog headers modified via
  # configure_file, etc, to use andgle bracket includes or quoted
  # includes depending on how whether standalone or bundled fmt is
  # being used. This is not implemented.
  # 
  if( MSVC )
    add_compile_options(/I ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)
  else()
    add_compile_options(-I${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)
  endif()

  #
  # fmt detects when it's included as a subdirectory and avoids
  # installing header files, so they have to be installed
  # explicitly. Install fmt headers under fmtlog to avoid conflicts
  # with a standalone fmt installation.
  #
  install(DIRECTORY fmt/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fmtlog/fmt)
  
  #
  # Place fmt libraries under fmtlog to avoid conflicts with a
  # standalone fmt installation. This is not consistent with fmtlog
  # libraries placed directly into the library installation directory,
  # but it makes standalone fmtlog installations simpler. 
  #
  install(TARGETS fmt DESTINATION ${CMAKE_INSTALL_LIBDIR}/fmtlog)
endif()

install(FILES fmtlog.h fmtlog-inl.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fmtlog)

if(BUILD_SHARED_LIBS)
  add_library(${FMTLOG_LIB_NAME} SHARED fmtlog.cc)
  target_link_libraries(${FMTLOG_LIB_NAME} PUBLIC fmt)
  install(TARGETS ${FMTLOG_LIB_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
else ()
  add_library(${FMTLOG_LIB_NAME} STATIC fmtlog.cc)
  install(TARGETS ${FMTLOG_LIB_NAME} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

set_target_properties(${FMTLOG_LIB_NAME} PROPERTIES DEBUG_POSTFIX d)

if(FMTLOG_BUILD_TEST)
  add_subdirectory(test)
endif()

#
# NOT MAINTAINED
#
if(FMTLOG_BUILD_BENCH)
  add_subdirectory(bench)
endif()
