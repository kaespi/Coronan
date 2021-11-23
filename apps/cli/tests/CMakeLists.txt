cmake_minimum_required(VERSION 3.15...3.20)

project(
  coronan_cli_app_tests
  VERSION 0.1
  LANGUAGES CXX)

add_executable(unittests_cli ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

add_executable(coronan::cli::unittests ALIAS unittests_cli)

set(UNITTEST_SOURCES)

find_package(sciplot CONFIG)
if(sciplot_FOUND)
  set(UNITTEST_SOURCES ${UNITTEST_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/plotting_sciplot_test.cpp
                                           ${CMAKE_CURRENT_LIST_DIR}/../plotting_sciplot.cpp)
endif()

target_sources(
  unittests_cli
  PRIVATE ${UNITTEST_SOURCES})

find_package(Catch2 REQUIRED CONFIG)

target_include_directories(unittests_cli PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(
  unittests_cli
  PRIVATE Catch2::Catch2
  PRIVATE coronan::library
  PRIVATE coronan::compile_warnings
  PRIVATE coronan::compile_options)

include(CTest)
include(Catch)

catch_discover_tests(
  unittests_cli
  TEST_PREFIX
  "unittest."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittest."
  OUTPUT_SUFFIX
  .xml)

add_custom_target(
  run_cli_unittests ALL
  COMMAND $<TARGET_FILE:coronan::cli::unittests>
  COMMENT "Run cli unittests")
