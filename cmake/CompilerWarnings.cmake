function(upf_enable_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
      $<$<BOOL:${UPF_DEMO_WARNINGS_AS_ERRORS}>:-Werror>)
  endif()
endfunction()
