/*
SecureNoteFS: A secure note taking file system
Implemented using libfuse3

Responsibilities of main()

Parse any CLI flags (for now, just CWD defaults).

ensure_directory("notes") & ensure_directory("data").

If data/*.tar.gz exists, pick the newest and tar_manager::extract().

Call fuse_main(), passing in fs::operations.

After fuse_main returns, call tar_manager::create_timestamped("data"), catch and warn on failure.

*/

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
