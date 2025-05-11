/*
SecureNoteFS: A secure note taking file system
Implemented using libfuse3

Responsibilities of main()

Parse any CLI flags (for now, just CWD defaults).

ensure_directory("notes") & ensure_directory("data").

If .tar.gz exists, pick the newest and tar_manager::extract().

Call fuse_main(), passing in fs::operations.

After fuse_main returns, call tar_manager::create_timestamped("data"), catch and warn on failure.

*/

#define FUSE_USE_VERSION 31

#include <iostream>
#include <filesystem>
#include "fs.hpp"

int main([[maybe_unused]] int argc, char const *argv[])
{
    std::cout << "Running on default from CWD" << '\n';
    std::filesystem::path current_working_dir = std::filesystem::current_path();

    // Create notes and data directories
    std::filesystem::create_directory("notes");
    std::filesystem::create_directory("data");

    auto sn_oper = get_sn_operations();
    int fuse_argc = 2;
    const char* fuse_argv[] = {
        argv[0],
        "notes", //Default mount point
        nullptr
    };

    return fuse_main(fuse_argc, const_cast<char**>(fuse_argv), sn_oper, nullptr);
}
