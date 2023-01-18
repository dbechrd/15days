cmake_minimum_required(VERSION 3.7)

# Disable dumb shit (ZERO_CHECK)
set(CMAKE_SUPPRESS_REGENERATION true)

#set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

project(15DAYS VERSION 0.0.1)

file (GLOB_RECURSE 15DAYS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME}
    #WIN32  # equivalent to /SUBSYSTEM:WINDOWS (will hide console)
    ${15DAYS_SOURCES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CRT_SECURE_NO_WARNINGS
    SDL_DISABLE_ANALYZE_MACROS
    WITH_SDL2_STATIC  # For SoLoud to use SDL2 as a back-end
)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-missing-braces
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-deprecated-declarations
        -Wno-misleading-indentation
        -Wno-writable-strings
    ) #-Wall -Wextra)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    #${CMAKE_SOURCE_DIR}/include/SDL
    ${CMAKE_SOURCE_DIR}/include/SDL3
)
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(${PROJECT_NAME}
    #SDL2main.lib
    #SDL2.lib
    SDL3.lib
    SDL3_ttf.lib
)