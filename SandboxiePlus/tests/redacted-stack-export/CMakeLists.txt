cmake_minimum_required(VERSION 3.16)
project(redacted_stack_export_tests CXX)

set(CMAKE_AUTOMOC ON)
enable_testing()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(SANDMAN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../SandMan")

add_executable(redacted_stack_export_test
    redacted_stack_export_test.cpp
    "${SANDMAN_DIR}/Helpers/RedactedStackExport.cpp"
    "${SANDMAN_DIR}/Helpers/RedactedStackExport.h")
set_target_properties(redacted_stack_export_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(redacted_stack_export_test PRIVATE "${SANDMAN_DIR}")
target_link_libraries(redacted_stack_export_test PRIVATE Qt${QT_VERSION_MAJOR}::Core)

foreach(case
    allow-list-privacy allow-list-fields
    opaque-ids-per-report same-name-distinct-modules module-name-shared
    frame-order-preserved unresolved-frame-marked partial-capture-preserved
    unicode-and-punctuation
    text-and-json-fidelity
    save-atomic-happy save-open-failure save-commit-failure save-existing-file
    save-does-not-touch-input concurrent-transforms-do-not-share-ids)
    add_test(NAME redacted_${case} COMMAND redacted_stack_export_test ${case})
    set_tests_properties(redacted_${case} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
