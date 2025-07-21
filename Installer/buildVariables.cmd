@echo on

set "qt_version=5.15.16"
set "qt6_version=6.9.1"
set "openssl_version=3.4.0"
set "ghSsl_user=xanasoft"
set "ghSsl_repo=openssl-builds"
set "ghQtBuilds_user=xanasoft"
set "ghQtBuilds_repo=qt-builds"
set "ghQtBuilds_hash_x86=6b25cb24755b158e580e73b7889a659e093012cc3d6428d2bfbff8ebad238cf2"
set "ghQtBuilds_hash_x64=1846e26def03152df2c7f6d2ab6fb4eea5ff8fdcd42a08723e2b9523131b9f91"

REM catch build_qt6
set "allArgs=%*"
set "allArgsCatch=%allArgs:build_qt6=%"
if not "%~1" == "" (
    if not "%allArgs%" == "%allArgsCatch%" (
        set "qt_version=%qt6_version%"
    ) else (
        set "qt_version=%qt_version%"
    )
)
