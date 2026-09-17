cmake_minimum_required(VERSION 3.16)
project(ProxyTunnelsTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Widgets Test)
add_executable(proxy-tests
    tst_ProxyTunnels.cpp
    ../../ProxyTunnels/ProxyProfile.cpp ../../ProxyTunnels/ProxyProfile.h
    ../../ProxyTunnels/ProxyTunnel.cpp ../../ProxyTunnels/ProxyTunnel.h
    ../../ProxyTunnels/ProxyManager.cpp ../../ProxyTunnels/ProxyManager.h
    ../../ProxyTunnels/ProxyWindow.cpp ../../ProxyTunnels/ProxyWindow.h)
target_compile_definitions(proxy-tests PRIVATE PROXY_TUNNELS_STANDALONE)
target_link_libraries(proxy-tests PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
if(WIN32)
    target_link_libraries(proxy-tests PRIVATE crypt32 ws2_32 iphlpapi)
endif()
enable_testing()
add_test(NAME proxy-tests COMMAND proxy-tests)
set_tests_properties(proxy-tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
