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
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SDL_DISABLE_ANALYZE_MACROS
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(${PROJECT_NAME}
    SDL2main.lib
    SDL2.lib
    SDL2_ttf.lib
)