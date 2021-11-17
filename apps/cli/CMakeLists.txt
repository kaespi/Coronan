cmake_minimum_required(VERSION 3.15...3.20)

project(
  coronan_cli
  VERSION 0.2.0
  LANGUAGES CXX)

set(CORONAN_CLI_SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

find_package(lyra REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)

find_package(sciplot CONFIG)
if(sciplot_FOUND)
  add_definitions(-DUSE_SCIPLOT)
  set(CORONAN_CLI_SOURCES ${CORONAN_CLI_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/plotting_sciplot.cpp)
endif()

add_executable(coronan_cli ${CORONAN_CLI_SOURCES})

add_executable(coronan::coronan_cli ALIAS coronan_cli)
set_target_properties(coronan_cli PROPERTIES CXX_EXTENSIONS OFF OUTPUT_NAME
                                                                coronan)
target_link_libraries(
  coronan_cli
  PRIVATE bfg::lyra
  PRIVATE fmt::fmt
  PRIVATE coronan::library
  PRIVATE coronan::compile_warnings
  PRIVATE coronan::compile_options
  $<$<BOOL:sciplot_FOUND>:sciplot::sciplot>)

include(StaticAnalyzers)
enable_static_analysis(coronan_cli)

install(TARGETS coronan_cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                    COMPONENT Coronan_Runtime_CLI)
