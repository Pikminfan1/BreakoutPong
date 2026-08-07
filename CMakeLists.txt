cmake_minimum_required(VERSION 3.20)
project(BreakoutPong VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch SDL3 from GitHub (cross-platform, no manual install needed) ---
include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# --- Define executable ---
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ecs/EntityManager.cpp
    # Add more .cpp files as you create them
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# --- Platform-specific: suppress console window on Windows ---
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()