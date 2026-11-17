cmake_minimum_required(VERSION 3.16)
project(suite_rack VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
include(GNUInstallDirs)

set(HEARTH_DSP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/hearth-dsp"
    CACHE PATH "Path to the hearth-dsp checkout")
if(NOT EXISTS "${HEARTH_DSP_DIR}/include/hdsp/hdsp.hpp")
  message(FATAL_ERROR "hearth-dsp not found at ${HEARTH_DSP_DIR}")
endif()
include_directories("${HEARTH_DSP_DIR}/include")

enable_testing()

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

set(RACK_UI_SOURCES src/ui/panel.cpp)

add_executable(rack_dsp_test test/test_dsp.cpp)
target_include_directories(rack_dsp_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(rack_dsp_test PRIVATE cxx_std_17)
add_test(NAME rack_dsp COMMAND rack_dsp_test)

add_executable(rack_panel_test test/test_panel.cpp ${RACK_UI_SOURCES})
target_include_directories(rack_panel_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(rack_panel_test PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(rack_panel_test PRIVATE cxx_std_17)
add_test(NAME rack_panel COMMAND rack_panel_test)

add_executable(rack_ui_preview test/ui_preview.cpp ${RACK_UI_SOURCES})
target_include_directories(rack_ui_preview PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(rack_ui_preview PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(rack_ui_preview PRIVATE cxx_std_17)
