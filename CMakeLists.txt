cmake_minimum_required(VERSION 3.25)

project(SecureNoteFS VERSION 0.1
        DESCRIPTION "Encrypted note-taking FUSE filesystem"
        LANGUAGES C CXX) # Add C to allow examples to build and run

# ==============================================================
# General build settings
# ==============================================================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # aids VS Code IntelliSense

# Enable compiler warnings (treat as errors). Adjust for your toolchain.
if (MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Option toggle for tests
option(SECURENOTEFS_BUILD_TESTS "Build unit tests" OFF)

# ==============================================================
# Source targets
# ==============================================================

# Collect all C++ sources under src/  ─ change if you reorganise.
file(GLOB_RECURSE SECURENOTEFS_SOURCES CONFIGURE_DEPENDS
     "src/*.cpp"
     "src/*.cxx"
     "src/*.cc"
     "src/*.c")

add_executable(securenotefs ${SECURENOTEFS_SOURCES})

target_include_directories(securenotefs
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include)

# ─── External deps (uncomment when added) ──────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 fuse3 REQUIRED)
target_link_libraries(securenotefs PRIVATE ${FUSE3_LIBRARIES})


# ==============================================================
# Tests
# ==============================================================

if (SECURENOTEFS_BUILD_TESTS)
    enable_testing() # activates the ctest runner so you can type ctest after a build
    add_subdirectory(tests)
endif()

# ==============================================================
# Install
# ==============================================================

install(TARGETS securenotefs
        RUNTIME DESTINATION bin)
