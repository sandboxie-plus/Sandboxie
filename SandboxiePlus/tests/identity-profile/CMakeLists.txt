cmake_minimum_required(VERSION 3.16)
project(identity_profile_tests CXX)

set(CMAKE_AUTOMOC ON)
enable_testing()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(SANDMAN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../SandMan")

add_executable(identity_profile_test
    identity_profile_test.cpp
    "${SANDMAN_DIR}/Helpers/IdentityProfile.cpp"
    "${SANDMAN_DIR}/Helpers/IdentityProfile.h"
    "${SANDMAN_DIR}/Windows/IdentityProfilesDialog.cpp"
    "${SANDMAN_DIR}/Windows/IdentityProfilesDialog.h")
set_target_properties(identity_profile_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(identity_profile_test PRIVATE "${SANDMAN_DIR}")
target_link_libraries(identity_profile_test PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)

foreach(case
    normalize-device normalize-serial validate-invalid json-corrupt
    store-roundtrip store-atomic-failure store-list-skips-bad store-remove
    regenerate-explicit clone-new-id import-new-id export-atomic
    bind-apply bind-two-boxes bind-refuses-running bind-refuses-manual
    bind-stale-after-regenerate bind-diverged bind-missing bind-unbind
    bind-keeps-templates bind-write-failure host-untouched bind-deferred-commit
    dialog-new-edit dialog-clone-regenerate dialog-import-export dialog-delete-guard)
    add_test(NAME identity_${case} COMMAND identity_profile_test ${case})
    set_tests_properties(identity_${case} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
