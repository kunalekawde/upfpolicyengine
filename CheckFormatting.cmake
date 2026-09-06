file(GLOB_RECURSE UPF_FORMAT_FILES
  "${SOURCE_ROOT}/include/*.hpp"
  "${SOURCE_ROOT}/src/*.cpp"
  "${SOURCE_ROOT}/tests/*.cpp"
  "${SOURCE_ROOT}/tests/*.hpp")

foreach(file IN LISTS UPF_FORMAT_FILES)
  file(READ "${file}" content)
  if(content MATCHES "[ \t]+\n" OR content MATCHES "[ \t]+$")
    message(FATAL_ERROR "Trailing whitespace: ${file}")
  endif()
  if(content MATCHES "\t")
    message(FATAL_ERROR "Tab indentation: ${file}")
  endif()
endforeach()

message(STATUS "Formatting policy passed")
