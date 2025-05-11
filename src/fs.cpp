/*
Responsibilities of fs:

Initialize a struct fuse_operations with extern "C" trampoline functions.

In each callback (getattr, readdir, open, read, write, cleanup):

    Map the incoming path under notes/ to the real path under data/.
    For reads: decrypt bytes via crypto::decrypt_chunk().
    For writes: buffer/plaintext → crypto::encrypt_chunk() → write to disk.

*/