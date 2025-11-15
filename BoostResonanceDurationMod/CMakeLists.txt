cmake_minimum_required(VERSION 3.18)

set(TARGET BoostResonanceDurationMod)
project(${TARGET})

set(SRC_FILES
    "src/dllmain.cpp"
    "src/VentruePassive.cpp"
)

add_library(${TARGET} SHARED ${SRC_FILES})

# Define Unreal Engine build type
target_compile_definitions(${TARGET} PRIVATE 
    UE_BUILD_SHIPPING=1
    UE_BUILD_DEBUG=0
    UE_BUILD_DEVELOPMENT=0
    UE_BUILD_TEST=0
    UBT_COMPILED_PLATFORM=Windows
    PLATFORM_WINDOWS=1
    WITH_CASE_PRESERVING_NAME=1
    UNICODE
    _UNICODE
)

# Use precompiled SDK + project headers
target_include_directories(${TARGET} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${UE4SS_INCLUDE}
    ${UE4SS_DEPS}/Unreal
    ${UE4SS_DEPS}/fmt
)

target_link_libraries(${TARGET} PRIVATE 
    UE4SS.lib
    fmt.lib
)

# Set C++ standard and encoding
target_compile_features(${TARGET} PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(${TARGET} PRIVATE /utf-8)
endif()

# Set RPATH for finding UE4SS.dll at runtime (only for non-Windows platforms)
if(NOT WIN32)
    set_target_properties(${TARGET} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN/../.."
    )
endif()