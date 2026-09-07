cmake_minimum_required(VERSION 3.16)
project(IniWriteTests LANGUAGES CXX)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
enable_testing()

add_custom_command(
    OUTPUT ini_write_under_test.inc fixture_return.inc
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_fixture.py" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS generate_fixture.py ../../QSbieAPI/Sandboxie/SbieIni.cpp ../../SandMan/Windows/OptionsWindow.cpp
    VERBATIM
)
add_executable(ini_write_test ini_write_test.cpp ini_write_under_test.inc fixture_return.inc)
set_target_properties(ini_write_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(ini_write_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ini_write_test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
foreach(case list-delete list-append list-retry list-noop bool-delete bool-success checkbox-delete options-failure options-success options-reported-failure)
    add_test(NAME ini_write_${case} COMMAND ini_write_test ${case})
    set_tests_properties(ini_write_${case} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
