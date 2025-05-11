# Learning notes from implementing this project

## How to run example hello.c

~~~Bash
# Create mount directory and run executable against it
mkdir -p /tmp/mnt
./build/securenotefs /tmp/mnt &

# List the contents
ls /tmp/mnt
# In the case of hello.c, you should see the single "hello" file

# Read the file
cat /tmp/mnt/hello

# Unmount cleanly and stop background FUSE process
fusermount3 -u /tmp/mnt
~~~


## Cmake

- Cmake is a meta-build system: you write one script (CMakeLists.txt) and Cmake converts it into the native build files your platform likes such as - GNU MakeFile, Ninja files, Visual Studio solution - so you never hand-write those platoform files again.

- Commands are functions: `command(arg1)`

- Variables use `${VAR}` to expand: like Bash but with braces - `${PROJECT_SOURCE_DIR}`

- Lists are semicolon-delimited: `a;b;c`

- Conditionals use variables that become auto-true: any variable that's non-empty is treated as "true"

- the `project()` commmand defies global metadata that IDE's use when they generate their own project files

- Two-liner that says "compile in C++23 mode and fail if the compiler can't":

~~~bash
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
~~~

- Tells CMake to emit compile_commands.json (one per .cpp compilation) so IntelliSense can read and know flags you use: `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)`

- MSVC is a built-in boolean variable that is set wen the compiler is Microsfot Visual V++. It is empty on Linux/Mac so the else branch will run in my project for the default of the GCC/Clang equivalent
~~~Bash
if (MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
~~~

- `option()` creates a cache enry a user can change at configure time with -DSECURENOTEFS_BUILD_TESTS=OFF: `option(SECURENOTEFS_BUILD_TESTS "Build unit tests" ON)`

- GLOB_RECURSE searches every sub-directory of src/ for files matching those patterns
- Configure depends means re-run CMake automatically if new files matching these patterns appear
~~~Bash
file(GLOB_RECURSE SECURENOTEFS_SOURCES CONFIGURE_DEPENDS
     "src/*.cpp" "src/*.cxx" "src/*.cc")
~~~
