cmake_minimum_required(VERSION 3.16)
project(suite_rack VERSION 0.36.0 LANGUAGES CXX)

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
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rack_dsp_test PRIVATE -Wall -Wextra -Werror=switch)
endif()
add_test(NAME rack_dsp COMMAND rack_dsp_test)

add_executable(rack_state_test test/test_state.cpp)
target_include_directories(rack_state_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(rack_state_test PRIVATE cxx_std_17)
add_test(NAME rack_state COMMAND rack_state_test)

add_executable(rack_panel_test test/test_panel.cpp ${RACK_UI_SOURCES})
target_include_directories(rack_panel_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(rack_panel_test PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(rack_panel_test PRIVATE cxx_std_17)
add_test(NAME rack_panel COMMAND rack_panel_test)

add_executable(rack_ui_preview test/ui_preview.cpp ${RACK_UI_SOURCES})
target_include_directories(rack_ui_preview PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(rack_ui_preview PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(rack_ui_preview PRIVATE cxx_std_17)

# ---------------------------------------------------------------------------
# Metadata generator: emits lv2/*.ttl from src/params.hpp and src/presets.hpp.
# The test fails if the committed TTL files are stale.
# ---------------------------------------------------------------------------
add_executable(rack_ttl_gen tools/gen_ttl.cpp)
target_include_directories(rack_ttl_gen PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(rack_ttl_gen PRIVATE cxx_std_17)
add_test(NAME rack_ttl
  COMMAND rack_ttl_gen --check --root "${CMAKE_SOURCE_DIR}")

# ---------------------------------------------------------------------------
# CLAP plugin
# ---------------------------------------------------------------------------
set(CLAP_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/clap"
    CACHE PATH "Directory where CLAP bundles are installed")
set(LV2_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lv2"
    CACHE PATH "Directory where LV2 bundles are installed")

pkg_check_modules(LV2 REQUIRED IMPORTED_TARGET lv2)

add_library(suite_rack MODULE src/clap/rack_clap.cpp ${RACK_UI_SOURCES})
target_include_directories(suite_rack PRIVATE
  "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(suite_rack PRIVATE
  PkgConfig::CAIRO PkgConfig::FREETYPE PkgConfig::X11 Threads::Threads
  ${CMAKE_DL_LIBS})
target_compile_features(suite_rack PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(suite_rack PRIVATE -Wall -Wextra -Werror=switch
                         -fvisibility=hidden)
endif()
set_target_properties(suite_rack PROPERTIES
  PREFIX "" OUTPUT_NAME "SuiteRack" SUFFIX ".clap"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/clap")

set(RACK_RESOURCE_FILES
  "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
  "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt")

add_custom_target(suite_rack_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/clap/SuiteRack.resources"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RACK_RESOURCE_FILES}
          "${CMAKE_BINARY_DIR}/clap/SuiteRack.resources/"
  DEPENDS suite_rack ${RACK_RESOURCE_FILES})

install(TARGETS suite_rack LIBRARY DESTINATION "${CLAP_DIR}")
install(FILES ${RACK_RESOURCE_FILES}
        DESTINATION "${CLAP_DIR}/SuiteRack.resources")


# ---------------------------------------------------------------------------
# LV2 build: same DSP and panel as the CLAP, exposed as an LV2 plugin + X11 UI.
# ---------------------------------------------------------------------------
set(LV2_BUNDLE_DIR "${CMAKE_BINARY_DIR}/lv2/suite-rack.lv2")

add_library(suite_rack_lv2 MODULE src/lv2/rack_lv2.cpp)
target_include_directories(suite_rack_lv2 PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(suite_rack_lv2 PRIVATE PkgConfig::LV2)
target_compile_features(suite_rack_lv2 PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(suite_rack_lv2 PRIVATE -Wall -Wextra -Werror=switch)
endif()
set_target_properties(suite_rack_lv2 PROPERTIES
  PREFIX ""
  OUTPUT_NAME "suite_rack"
  LIBRARY_OUTPUT_DIRECTORY "${LV2_BUNDLE_DIR}")

add_library(suite_rack_lv2_ui MODULE src/ui/lv2_ui.cpp ${RACK_UI_SOURCES})
target_include_directories(suite_rack_lv2_ui PRIVATE
  "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(suite_rack_lv2_ui PRIVATE
  PkgConfig::CAIRO PkgConfig::FREETYPE PkgConfig::X11
  PkgConfig::LV2 Threads::Threads)
target_compile_features(suite_rack_lv2_ui PRIVATE cxx_std_17)
set_target_properties(suite_rack_lv2_ui PROPERTIES
  PREFIX ""
  OUTPUT_NAME "suite_rack_ui"
  LIBRARY_OUTPUT_DIRECTORY "${LV2_BUNDLE_DIR}")

add_custom_target(suite_rack_lv2_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${LV2_BUNDLE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/lv2/manifest.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/suite_rack.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/presets.ttl"
          "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
          "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt"
          "${LV2_BUNDLE_DIR}/"
  DEPENDS suite_rack_lv2 suite_rack_lv2_ui)

add_custom_target(dist
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E tar czf
          "${CMAKE_BINARY_DIR}/dist/suite-rack-${PROJECT_VERSION}-linux-x86_64.tar.gz"
          lv2/suite-rack.lv2 clap/SuiteRack.clap clap/SuiteRack.resources
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  DEPENDS suite_rack_lv2_bundle suite_rack_bundle
  COMMENT "Packing suite-rack-${PROJECT_VERSION}")

install(TARGETS suite_rack_lv2 suite_rack_lv2_ui
        LIBRARY DESTINATION "${LV2_DIR}/suite-rack.lv2")
install(FILES
  lv2/manifest.ttl
  lv2/suite_rack.ttl
  lv2/presets.ttl
  assets/fonts/Jost-ExtraBold.ttf
  assets/fonts/OFL.txt
  DESTINATION "${LV2_DIR}/suite-rack.lv2")
