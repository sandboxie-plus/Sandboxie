@echo on

set "qt_version=6.8.4"
set "qt6_version=6.8.3"
rem CodeQL uses stock Qt, pinned to a version with public MSVC binaries.
rem Do not track custom or commercial LTS patch releases automatically:
rem install-qt-action may not be able to download those versions.
set "qt_codeql_version=6.8.3"
set "openssl_version=3.4.0"
set "ghSsl_user=xanasoft"
set "ghSsl_repo=openssl-builds"
set "ghQtBuilds_user=xanasoft"
set "ghQtBuilds_repo=qt-builds"
rem set "ghQtBuilds_hash_x86=6b25cb24755b158e580e73b7889a659e093012cc3d6428d2bfbff8ebad238cf2"
set "ghQtBuilds_hash_x64=94e56b85a640a785415fbd2cacf70be341d16ce09fd96d98bce7e369c933ebc9"

rem qt_version is the custom windows 7 capable build used for x86/x64,
rem qt6_version is the stock qt used for ARM64, there is no windows 7 on ARM64.
rem the archive names change between qt releases, keep them in one place.
rem x64 is the release only build, it carries a -release suffix and ships no debug libs.
set "ghQtBuilds_file_x86=qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86.7z"
set "ghQtBuilds_file_x64=qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86_64-release.7z"
