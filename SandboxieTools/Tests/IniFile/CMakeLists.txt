cmake_minimum_required(VERSION 3.16)
project(IniFileTests LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "These tests use the production Windows CIniFile implementation.")
endif()

set(SANDBOXIE_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH
    "Sandboxie source tree containing Sandboxie/common/ini.cpp")
get_filename_component(SANDBOXIE_SOURCE_ROOT "${SANDBOXIE_SOURCE_ROOT}" ABSOLUTE)

add_executable(ini_file_test
    ini_file_test.cpp
    "${SANDBOXIE_SOURCE_ROOT}/Sandboxie/common/ini.cpp"
)
target_compile_features(ini_file_test PRIVATE cxx_std_17)
target_compile_definitions(ini_file_test PRIVATE UNICODE _UNICODE)
target_include_directories(ini_file_test PRIVATE "${SANDBOXIE_SOURCE_ROOT}/Sandboxie/common")
if(MSVC)
    target_compile_options(ini_file_test PRIVATE /utf-8 /EHsc)
endif()

enable_testing()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-data")
add_test(NAME ini_file_values
    COMMAND ini_file_test "${CMAKE_CURRENT_BINARY_DIR}/test-data")
