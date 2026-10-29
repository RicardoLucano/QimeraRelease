cmake_minimum_required(VERSION 3.15)
project(QWebRtcPluginTests C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(qwebrtc_plugin_tests
    test_main.cpp
    test_plugin_api.cpp
)

# Reference headers from the nested ThirdParty module
target_include_directories(qwebrtc_plugin_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/ThirdParty/WebRTC/Include"
)

# Link imported library webrtc_c_wrapper.lib
target_link_libraries(qwebrtc_plugin_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/ThirdParty/WebRTC/Lib/win64/webrtc_c_wrapper.lib"
)

# Set static CRT linking
set_property(TARGET qwebrtc_plugin_tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
