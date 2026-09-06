cmake_minimum_required(VERSION 3.16)
project(upf_rule_pipeline_demo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UPF_DEMO_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(UPF_DEMO_COVERAGE "Enable coverage" OFF)
option(UPF_DEMO_BUILD_FUZZERS "Build fuzz target" OFF)
option(UPF_DEMO_CLANG_TIDY "Run clang-tidy during compilation" OFF)
set(UPF_DEMO_SANITIZERS "" CACHE STRING "Comma-separated sanitizers")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UPF_DEMO_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE clang-tidy REQUIRED)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d)
FetchContent_Declare(Catch2 URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.10.tar.gz
  URL_HASH SHA256=d54a712b7b1d7708bc7a819a8e6e47b2fde9536f487b89ccbca295072a7d9943)
FetchContent_MakeAvailable(json Catch2)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Coverage.cmake)

file(GLOB_RECURSE UPF_CORE_SOURCES CONFIGURE_DEPENDS src/domain/*.cpp src/application/*.cpp src/adapters/*.cpp)
add_library(upf_core ${UPF_CORE_SOURCES})
target_include_directories(upf_core PUBLIC include PRIVATE src)
target_link_libraries(upf_core PUBLIC nlohmann_json::nlohmann_json)
target_compile_features(upf_core PUBLIC cxx_std_11)
upf_enable_warnings(upf_core)
upf_enable_sanitizers(upf_core)
upf_enable_coverage(upf_core)

add_executable(upf-rule-pipeline-demo src/cli/main.cpp)
target_link_libraries(upf-rule-pipeline-demo PRIVATE upf_core)
target_compile_features(upf-rule-pipeline-demo PRIVATE cxx_std_11)
upf_enable_warnings(upf-rule-pipeline-demo)
upf_enable_sanitizers(upf-rule-pipeline-demo)
upf_enable_coverage(upf-rule-pipeline-demo)

enable_testing()
add_subdirectory(tests)

add_custom_target(format-check
  COMMAND ${CMAKE_COMMAND} -DSOURCE_ROOT=${PROJECT_SOURCE_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/CheckFormatting.cmake
  VERBATIM)

if(UPF_DEMO_COVERAGE)
  find_program(GCOVR_EXE gcovr)
  if(GCOVR_EXE)
    add_custom_target(coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXE} --root ${PROJECT_SOURCE_DIR}
              --filter ${PROJECT_SOURCE_DIR}/src/domain/
              --filter ${PROJECT_SOURCE_DIR}/src/application/
              --txt-metric branch --exclude-throw-branches --exclude-unreachable-branches
              --fail-under-branch 90
      DEPENDS upf_tests
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()
endif()
