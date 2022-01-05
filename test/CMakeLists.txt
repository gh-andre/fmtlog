if(NOT MSVC)
  add_library(static_lib STATIC lib.cc)

  add_library(static_header_lib STATIC lib.cc)
  target_compile_definitions(static_header_lib PUBLIC FMTLOG_HEADER_ONLY FMT_HEADER_ONLY=1)

  #
  # shared_lib is a test library and it's built regardless whether
  # fmtlog is built as a shared library (i.e. BUILD_SHARED_LIBS=ON).
  #
  # This library is self-contained and it is assumed that fmt is
  # linked into fmtlog is being built as a shared library. Otherwise
  # we need to include both static libraries to link.
  #
  add_library(shared_lib SHARED lib.cc)
  if( BUILD_SHARED_LIBS )
    target_link_libraries(shared_lib PUBLIC fmtlog)
  else()
    target_link_libraries(shared_lib PUBLIC fmtlog fmt)
  endif()
  install(TARGETS shared_lib)

  add_library(shared_header_lib SHARED lib.cc)
  target_compile_definitions(shared_header_lib PUBLIC FMTLOG_HEADER_ONLY FMT_HEADER_ONLY=1)
  if( BUILD_SHARED_LIBS )
    target_link_libraries(shared_lib INTERFACE fmtlog)
  endif()
  install(TARGETS shared_header_lib)

  #
  # All conditional sections below are intentionally not collapsed
  # into two to keep the source in sync with the original code, so
  # it's easier to merge.
  #
  # The first term following `link_` describes the type of the
  # fmtlog library and the second describes the type of the test
  # library. All cases of fmtlog included as a static library
  # require fmt included explicitly. Otherwise, fmt will be pulled
  # via a library reference in libfmtlog.so.
  #

  if( NOT BUILD_SHARED_LIBS )
    add_executable(link_static_static link_test.cc)
    target_link_libraries(link_static_static static_lib fmtlog fmt)
    install(TARGETS link_static_static)
  endif()

  if( NOT BUILD_SHARED_LIBS )
    add_executable(link_static_shared link_test.cc)
    target_link_libraries(link_static_shared shared_lib)
    install(TARGETS link_static_shared)
  endif()

  if( BUILD_SHARED_LIBS )
    add_executable(link_shared_static link_test.cc)
    target_link_libraries(link_shared_static static_lib fmtlog)
    install(TARGETS link_shared_static)
  endif()

  if( BUILD_SHARED_LIBS )
    add_executable(link_shared_shared link_test.cc)
    target_link_libraries(link_shared_shared shared_lib fmtlog)
    install(TARGETS link_shared_shared)
  endif()

  if( NOT BUILD_SHARED_LIBS )
    add_executable(link_header_static link_test.cc)
    target_link_libraries(link_header_static static_header_lib)
    install(TARGETS link_header_static)
  endif()

  add_executable(link_header_shared link_test.cc)
  target_link_libraries(link_header_shared shared_header_lib)
  install(TARGETS link_header_shared)

endif()

add_executable(log_test log_test.cc)
target_link_libraries(log_test fmtlog fmt)
#target_compile_definitions(log_test PUBLIC FMTLOG_HEADER_ONLY)
install(TARGETS log_test)

add_executable(enc_dec_test enc_dec_test.cc)
target_link_libraries(enc_dec_test fmtlog fmt)
install(TARGETS enc_dec_test)

add_executable(multithread_test multithread_test.cc)
target_link_libraries(multithread_test fmtlog fmt)
install(TARGETS multithread_test)

