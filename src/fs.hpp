#ifndef SECURENOTEFS_FS_HPP
#define SECURENOTEFS_FS_HPP

#include <fuse3/fuse.h>
#include <sys/statvfs.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialization callback: tweak cache and feature flags
void* sn_init(struct fuse_conn_info* conn, struct fuse_config* cfg);

// File attribute lookup (stat)
int sn_getattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi);

// Access checks (permissions)
int sn_access(const char* path, int mask);

// Read symbolic link target
int sn_readlink(const char* path, char* buf, size_t size);

// Read directory entries
int sn_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info* fi,
               enum fuse_readdir_flags flags);

// Create special or regular file
int sn_mknod(const char* path, mode_t mode, dev_t rdev);

// Create directory
int sn_mkdir(const char* path, mode_t mode);

// Remove file
int sn_unlink(const char* path);

// Remove directory
int sn_rmdir(const char* path);

// Create symbolic link
int sn_symlink(const char* from, const char* to);

// Rename a file or directory
int sn_rename(const char* from, const char* to, unsigned int flags);

// Create hard link
int sn_link(const char* from, const char* to);

// Change permissions
int sn_chmod(const char* path, mode_t mode, struct fuse_file_info* fi);

// Change ownership
int sn_chown(const char* path, uid_t uid, gid_t gid, struct fuse_file_info* fi);

// Truncate or extend file size
int sn_truncate(const char* path, off_t size, struct fuse_file_info* fi);

#ifdef HAVE_UTIMENSAT
// Change file timestamps
int sn_utimens(const char* path, const struct timespec ts[2], struct fuse_file_info* fi);
#endif

// Create and open file
int sn_create(const char* path, mode_t mode, struct fuse_file_info* fi);

// Open existing file
int sn_open(const char* path, struct fuse_file_info* fi);

// Read data from file
int sn_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);

// Write data to file
int sn_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);

// Get filesystem statistics
int sn_statfs(const char* path, struct statvfs* stbuf);

// Release an open file
int sn_release(const char* path, struct fuse_file_info* fi);

// Synchronize file contents
int sn_fsync(const char* path, int isdatasync, struct fuse_file_info* fi);

#ifdef HAVE_POSIX_FALLOCATE
// Allocate or deallocate file space
int sn_fallocate(const char* path, int mode, off_t offset, off_t length, struct fuse_file_info* fi);
#endif

#ifdef HAVE_SETXATTR
// Extended attribute operations
int sn_setxattr(const char* path, const char* name, const char* value, size_t size, int flags);
int sn_getxattr(const char* path, const char* name, char* value, size_t size);
int sn_listxattr(const char* path, char* list, size_t size);
int sn_removexattr(const char* path, const char* name);
#endif

#ifdef HAVE_COPY_FILE_RANGE
// Optimized file copying
ssize_t sn_copy_file_range(const char* path_in, struct fuse_file_info* fi_in,
                           off_t offset_in, const char* path_out,
                           struct fuse_file_info* fi_out,
                           off_t offset_out, size_t len, int flags);
#endif

// Seek within a file
off_t sn_lseek(const char* path, off_t off, int whence, struct fuse_file_info* fi);

// Returns pointer to populated operations struct
const struct fuse_operations* get_sn_operations(void);

#ifdef __cplusplus
}
#endif

#endif // SECURENOTEFS_FS_HPP
