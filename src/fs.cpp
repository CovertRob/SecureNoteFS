/*
Responsibilities of fs:

Initialize a struct fuse_operations with extern "C" trampoline functions.

In each callback (getattr, readdir, open, read, write, cleanup):

    Map the incoming path under notes/ to the real path under data/.
    For reads: decrypt bytes via crypto::decrypt_chunk().
    For writes: buffer/plaintext → crypto::encrypt_chunk() → write to disk.

*/

/** @file
 *
 * This file system mirrors the existing file system hierarchy of the
 * system, starting at the root file system. This is implemented by
 * just "passing through" all requests to the corresponding user-space
 * libc functions. Its performance is terrible.
 *
 * Compile with
 *
 *     gcc -Wall passthrough.c `pkg-config fuse3 --cflags --libs` -o passthrough
 *
 * ## Source code ##
 * \include passthrough.c
 */

 #define FUSE_USE_VERSION 31

//  #define _GNU_SOURCE
 
 #ifdef linux
 /* For pread()/pwrite()/utimensat() */
 #define _XOPEN_SOURCE 700
 #endif
 
 #include <fuse3/fuse.h>
 #include "fs.hpp"
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <dirent.h>
 #include <errno.h>
 #ifdef __FreeBSD__
 #include <sys/socket.h>
 #include <sys/un.h>
 #endif
 #include <sys/time.h>
 #ifdef HAVE_SETXATTR
 #include <sys/xattr.h>
 #endif
 
 #include "utils.hpp" //Previously passthrough_helpers.h
 
 extern "C" {

    int fill_dir_plus = 0;
    
    void *sn_init(struct fuse_conn_info *conn,
                struct fuse_config *cfg)
    {
        (void) conn;
        cfg->use_ino = 1;
    
        /* parallel_direct_writes feature depends on direct_io features.
            To make parallel_direct_writes valid, need either set cfg->direct_io
            in current function (recommended in high level API) or set fi->direct_io
            in xmp_create() or xmp_open(). */
        // cfg->direct_io = 1;
            cfg->parallel_direct_writes = 1;
    
        /* Pick up changes from lower filesystem right away. This is
            also necessary for better hardlink support. When the kernel
            calls the unlink() handler, it does not know the inode of
            the to-be-removed entry and can therefore not invalidate
            the cache of the associated inode - resulting in an
            incorrect st_nlink value being reported for any remaining
            hardlinks to this inode. */
        if (!cfg->auto_cache) {
            cfg->entry_timeout = 0;
            cfg->attr_timeout = 0;
            cfg->negative_timeout = 0;
        }
    
        return NULL;
    }

    int sn_getattr(const char *path, struct stat *stbuf,
                    struct fuse_file_info *fi)
    {
        (void) fi;
        int res;
    
        res = lstat(path, stbuf);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_access(const char *path, int mask)
    {
        int res;
    
        res = access(path, mask);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_readlink(const char *path, char *buf, size_t size)
    {
        int res;
    
        res = readlink(path, buf, size - 1);
        if (res == -1)
            return -errno;
    
        buf[res] = '\0';
        return 0;
    }
    
    
    int sn_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi,
                    enum fuse_readdir_flags flags)
    {
        DIR *dp;
        struct dirent *de;
    
        (void) offset;
        (void) fi;
        (void) flags;
    
        dp = opendir(path);
        if (dp == NULL)
            return -errno;
    
        while ((de = readdir(dp)) != NULL) {
            struct stat st;
            if (fill_dir_plus) {
                fstatat(dirfd(dp), de->d_name, &st,
                    AT_SYMLINK_NOFOLLOW);
            } else {
                memset(&st, 0, sizeof(st));
                st.st_ino = de->d_ino;
                st.st_mode = de->d_type << 12;
            }
            if (filler(buf, de->d_name, &st, 0, FUSE_FILL_DIR_PLUS))
                break;
        }
    
        closedir(dp);
        return 0;
    }
    
    int sn_mknod(const char *path, mode_t mode, dev_t rdev)
    {
        int res;
    
        res = mknod_wrapper(AT_FDCWD, path, NULL, mode, rdev);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_mkdir(const char *path, mode_t mode)
    {
        int res;
    
        res = mkdir(path, mode);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_unlink(const char *path)
    {
        int res;
    
        res = unlink(path);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_rmdir(const char *path)
    {
        int res;
    
        res = rmdir(path);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_symlink(const char *from, const char *to)
    {
        int res;
    
        res = symlink(from, to);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_rename(const char *from, const char *to, unsigned int flags)
    {
        int res;
    
        if (flags)
            return -EINVAL;
    
        res = rename(from, to);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_link(const char *from, const char *to)
    {
        int res;
    
        res = link(from, to);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_chmod(const char *path, mode_t mode,
                struct fuse_file_info *fi)
    {
        (void) fi;
        int res;
    
        res = chmod(path, mode);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_chown(const char *path, uid_t uid, gid_t gid,
                struct fuse_file_info *fi)
    {
        (void) fi;
        int res;
    
        res = lchown(path, uid, gid);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_truncate(const char *path, off_t size,
                struct fuse_file_info *fi)
    {
        int res;
    
        if (fi != NULL)
            res = ftruncate(fi->fh, size);
        else
            res = truncate(path, size);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    #ifdef HAVE_UTIMENSAT
    int sn_utimens(const char *path, const struct timespec ts[2],
                    struct fuse_file_info *fi)
    {
        (void) fi;
        int res;
    
        /* don't use utime/utimes since they follow symlinks */
        res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    #endif
    
    int sn_create(const char *path, mode_t mode,
                struct fuse_file_info *fi)
    {
        int res;
    
        res = open(path, fi->flags, mode);
        if (res == -1)
            return -errno;
    
        fi->fh = res;
        return 0;
    }
    
    int sn_open(const char *path, struct fuse_file_info *fi)
    {
        int res;
    
        res = open(path, fi->flags);
        if (res == -1)
            return -errno;
    
            /* Enable direct_io when open has flags O_DIRECT to enjoy the feature
            parallel_direct_writes (i.e., to get a shared lock, not exclusive lock,
            for writes to the same file). */
        if (fi->flags & O_DIRECT) {
            fi->direct_io = 1;
            fi->parallel_direct_writes = 1;
        }
    
        fi->fh = res;
        return 0;
    }
    
    int sn_read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi)
    {
        int fd;
        int res;
    
        if(fi == NULL)
            fd = open(path, O_RDONLY);
        else
            fd = fi->fh;
        
        if (fd == -1)
            return -errno;
    
        res = pread(fd, buf, size, offset);
        if (res == -1)
            res = -errno;
    
        if(fi == NULL)
            close(fd);
        return res;
    }
    
    int sn_write(const char *path, const char *buf, size_t size,
                off_t offset, struct fuse_file_info *fi)
    {
        int fd;
        int res;
    
        (void) fi;
        if(fi == NULL)
            fd = open(path, O_WRONLY);
        else
            fd = fi->fh;
        
        if (fd == -1)
            return -errno;
    
        res = pwrite(fd, buf, size, offset);
        if (res == -1)
            res = -errno;
    
        if(fi == NULL)
            close(fd);
        return res;
    }
    
    int sn_statfs(const char *path, struct statvfs *stbuf)
    {
        int res;
    
        res = statvfs(path, stbuf);
        if (res == -1)
            return -errno;
    
        return 0;
    }
    
    int sn_release(const char *path, struct fuse_file_info *fi)
    {
        (void) path;
        close(fi->fh);
        return 0;
    }
    
    int sn_fsync(const char *path, int isdatasync,
                struct fuse_file_info *fi)
    {
        /* Just a stub.	 This method is optional and can safely be left
            unimplemented */
    
        (void) path;
        (void) isdatasync;
        (void) fi;
        return 0;
    }
    
    #ifdef HAVE_POSIX_FALLOCATE
    int sn_fallocate(const char *path, int mode,
                off_t offset, off_t length, struct fuse_file_info *fi)
    {
        int fd;
        int res;
    
        (void) fi;
    
        if (mode)
            return -EOPNOTSUPP;
    
        if(fi == NULL)
            fd = open(path, O_WRONLY);
        else
            fd = fi->fh;
        
        if (fd == -1)
            return -errno;
    
        res = -posix_fallocate(fd, offset, length);
    
        if(fi == NULL)
            close(fd);
        return res;
    }
    #endif
    
    #ifdef HAVE_SETXATTR
    /* xattr operations are optional and can safely be left unimplemented */
    int sn_setxattr(const char *path, const char *name, const char *value,
                size_t size, int flags)
    {
        int res = lsetxattr(path, name, value, size, flags);
        if (res == -1)
            return -errno;
        return 0;
    }
    
    int sn_getxattr(const char *path, const char *name, char *value,
                size_t size)
    {
        int res = lgetxattr(path, name, value, size);
        if (res == -1)
            return -errno;
        return res;
    }
    
    int sn_listxattr(const char *path, char *list, size_t size)
    {
        int res = llistxattr(path, list, size);
        if (res == -1)
            return -errno;
        return res;
    }
    
    int sn_removexattr(const char *path, const char *name)
    {
        int res = lremovexattr(path, name);
        if (res == -1)
            return -errno;
        return 0;
    }
    #endif /* HAVE_SETXATTR */
    
    #ifdef HAVE_COPY_FILE_RANGE
    ssize_t sn_copy_file_range(const char *path_in,
                        struct fuse_file_info *fi_in,
                        off_t offset_in, const char *path_out,
                        struct fuse_file_info *fi_out,
                        off_t offset_out, size_t len, int flags)
    {
        int fd_in, fd_out;
        ssize_t res;
    
        if(fi_in == NULL)
            fd_in = open(path_in, O_RDONLY);
        else
            fd_in = fi_in->fh;
    
        if (fd_in == -1)
            return -errno;
    
        if(fi_out == NULL)
            fd_out = open(path_out, O_WRONLY);
        else
            fd_out = fi_out->fh;
    
        if (fd_out == -1) {
            close(fd_in);
            return -errno;
        }
    
        res = copy_file_range(fd_in, &offset_in, fd_out, &offset_out, len,
                    flags);
        if (res == -1)
            res = -errno;
    
        if (fi_out == NULL)
            close(fd_out);
        if (fi_in == NULL)
            close(fd_in);
    
        return res;
    }
    #endif
    
    off_t sn_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi)
    {
        int fd;
        off_t res;
    
        if (fi == NULL)
            fd = open(path, O_RDONLY);
        else
            fd = fi->fh;
    
        if (fd == -1)
            return -errno;
    
        res = lseek(fd, off, whence);
        if (res == -1)
            res = -errno;
    
        if (fi == NULL)
            close(fd);
        return res;
    }
    
    // const struct fuse_operations sn_oper = {
    //     .init           = sn_init,
    //     .getattr	= sn_getattr,
    //     .access		= sn_access,
    //     .readlink	= sn_readlink,
    //     .readdir	= sn_readdir,
    //     .mknod		= sn_mknod,
    //     .mkdir		= sn_mkdir,
    //     .symlink	= sn_symlink,
    //     .unlink		= sn_unlink,
    //     .rmdir		= sn_rmdir,
    //     .rename		= sn_rename,
    //     .link		= sn_link,
    //     .chmod		= sn_chmod,
    //     .chown		= sn_chown,
    //     .truncate	= sn_truncate,
    // #ifdef HAVE_UTIMENSAT
    //     .utimens	= sn_utimens,
    // #endif
    //     .open		= sn_open,
    //     .create 	= sn_create,
    //     .read		= sn_read,
    //     .write		= sn_write,
    //     .statfs		= sn_statfs,
    //     .release	= sn_release,
    //     .fsync		= sn_fsync,
    // #ifdef HAVE_POSIX_FALLOCATE
    //     .fallocate	= sn_fallocate,
    // #endif
    // #ifdef HAVE_SETXATTR
    //     .setxattr	= sn_setxattr,
    //     .getxattr	= sn_getxattr,
    //     .listxattr	= sn_listxattr,
    //     .removexattr	= sn_removexattr,
    // #endif
    // #ifdef HAVE_COPY_FILE_RANGE
    //     .copy_file_range = sn_copy_file_range,
    // #endif
    //     .lseek		= sn_lseek,
    // };

    // Remove the static initializer above. Instead, declare an empty struct:
    static struct fuse_operations sn_oper;

    // Populate it once at runtime:
    #include <cstring>  // for std::memset

    extern "C" const struct fuse_operations* get_sn_operations() {
        static bool initialized = false;
        if (!initialized) {
            // Zero out all callbacks (unused ones stay nullptr)
            std::memset(&sn_oper, 0, sizeof(sn_oper));

            // Wire up only the handlers you’ve implemented:
            sn_oper.init      = sn_init;
            sn_oper.getattr   = sn_getattr;
            sn_oper.access    = sn_access;
            sn_oper.readlink  = sn_readlink;
            sn_oper.readdir   = sn_readdir;
            sn_oper.mknod     = sn_mknod;
            sn_oper.mkdir     = sn_mkdir;
            sn_oper.unlink    = sn_unlink;
            sn_oper.rmdir     = sn_rmdir;
            sn_oper.symlink   = sn_symlink;
            sn_oper.rename    = sn_rename;
            sn_oper.link      = sn_link;
            sn_oper.chmod     = sn_chmod;
            sn_oper.chown     = sn_chown;
            sn_oper.truncate  = sn_truncate;
    #ifdef HAVE_UTIMENSAT
            sn_oper.utimens   = sn_utimens;
    #endif
            sn_oper.open      = sn_open;
            sn_oper.create    = sn_create;
            sn_oper.read      = sn_read;
            sn_oper.write     = sn_write;
            sn_oper.statfs    = sn_statfs;
            sn_oper.release   = sn_release;
            sn_oper.fsync     = sn_fsync;
    #ifdef HAVE_POSIX_FALLOCATE
            sn_oper.fallocate = sn_fallocate;
    #endif
    #ifdef HAVE_SETXATTR
            sn_oper.setxattr  = sn_setxattr;
            sn_oper.getxattr  = sn_getxattr;
            sn_oper.listxattr = sn_listxattr;
            sn_oper.removexattr = sn_removexattr;
    #endif
    #ifdef HAVE_COPY_FILE_RANGE
            sn_oper.copy_file_range = sn_copy_file_range;
    #endif
            sn_oper.lseek     = sn_lseek;

            initialized = true;
        }
        return &sn_oper;
    }
}