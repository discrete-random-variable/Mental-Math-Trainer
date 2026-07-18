cmake_minimum_required(VERSION 3.14)
project(adaptive-math-trainer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Dependencies ─────────────────────────────────────────────────────────────
# Fetch nlohmann/json via CMake FetchContent (modern dependency management)
include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(json)

# ─── Core Library ─────────────────────────────────────────────────────────────
set(CORE_SOURCES
    src/core/SkillDAG.cpp
    src/core/SkillManager.cpp
    src/core/AdaptiveSelector.cpp
    src/core/Session.cpp
)

set(STATISTICS_SOURCES
    src/statistics/Statistics.cpp
)

set(PERSISTENCE_SOURCES
    src/persistence/Persistence.cpp
)

set(UI_SOURCES
    src/ui/TerminalUI.cpp
)

add_library(adaptive_math_core
    ${CORE_SOURCES}
    ${STATISTICS_SOURCES}
    ${PERSISTENCE_SOURCES}
    ${UI_SOURCES}
)

target_include_directories(adaptive_math_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(adaptive_math_core
    PUBLIC
        nlohmann_json::nlohmann_json
)

# Compiler warnings for clean code
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(adaptive_math_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ─── Executable ───────────────────────────────────────────────────────────────
add_executable(adaptive-math-trainer src/main.cpp)
target_link_libraries(adaptive-math-trainer PRIVATE adaptive_math_core)

# ─── Install ──────────────────────────────────────────────────────────────────
install(TARGETS adaptive-math-trainer DESTINATION bin)
