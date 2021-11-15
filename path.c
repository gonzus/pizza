#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#include "path.h"

#define BUFFER_SIZE 4096

void path_build(Path* p) {
    buffer_build(&p->name);
}

void path_from_string(Path* p, const char* str, int len) {
    path_build(p);
    buffer_append_string(&p->name, str, len);
    buffer_null_terminate(&p->name);
    --p->name.len; // HACK ALARM
}

void path_from_slice(Path* p, Slice s) {
    path_build(p);
    buffer_append_slice(&p->name, s);
    buffer_null_terminate(&p->name);
    --p->name.len; // HACK ALARM
}

void path_destroy(Path* p) {
    buffer_destroy(&p->name);
}

int path_exists(Path* p, int* exists) {
    *exists = 0;
    int ret = 0;
    do {
       int a = access(p->name.ptr, F_OK);
       int e = errno;
       if (a < 0) {
           if (e != ENOENT) {
               // doesn't exist, flag error
               ret = e;
               break;
           }
       } else {
           *exists = 1;
       }
    } while (0);
    return ret;
}

#define CHECK_MODE(path, answer, check) \
    *answer = 0; \
    int ret = 0; \
    do { \
        struct stat sb; \
        if (lstat((path)->name.ptr, &sb) < 0) { \
            ret = errno; \
            break; \
        } \
        *answer = check(sb.st_mode); \
    } while (0); \
    return ret

int path_is_file(Path* p, int* is_file) {
    CHECK_MODE(p, is_file, S_ISREG);
}

int path_is_dir(Path* p, int* is_dir) {
    CHECK_MODE(p, is_dir, S_ISDIR);
}

int path_is_symlink(Path* p, int* is_symlink) {
    CHECK_MODE(p, is_symlink, S_ISLNK);
}

int path_is_fifo(Path* p, int* is_fifo) {
    CHECK_MODE(p, is_fifo, S_ISFIFO);
}

int path_is_socket(Path* p, int* is_socket) {
    CHECK_MODE(p, is_socket, S_ISSOCK);
}

int path_is_chardev(Path* p, int* is_chardev) {
    CHECK_MODE(p, is_chardev, S_ISCHR);
}

int path_is_blockdev(Path* p, int* is_blockdev) {
    CHECK_MODE(p, is_blockdev, S_ISBLK);
}

int path_readlink(Path* p, Buffer* b) {
    int ret = 0;
    do {
        char buf[PATH_MAX];
        int len = readlink(p->name.ptr, buf, PATH_MAX);
        if (len < 0) {
            ret = errno;
            break;
        }
        buffer_append_string(b, buf, len);
    } while (0);
    return ret;
}

int path_mkdir(Path* p) {
    int ret = 0;
    do {
        int exists = 0;
        ret = path_exists(p, &exists);
        if (ret) {
            // could not check existence, bail out
            break;
        }
        if (exists) {
            // path exists
            int is_dir = 0;
            ret = path_is_dir(p, &is_dir);
            if (ret) {
                // could not check if it is a directory, bail out
                break;
            }
            if (is_dir) {
                // path exists and is a directory
                if (utimes(p->name.ptr, 0) < 0) {
                    ret = errno;
                    break;
                }
            } else {
                // path exists and is not a directory
                ret = EEXIST;
                break;
            }
        } else {
            // path does not exist
            mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
            if (mkdir(p->name.ptr, mode) < 0) {
                ret = errno;
                break;
            }
        }
    } while (0);
    return ret;
}

int path_rmdir(Path* p) {
    int ret = 0;
    do {
        int exists = 0;
        ret = path_exists(p, &exists);
        if (ret) {
            // could not check existence, bail out
            break;
        }
        if (exists) {
            // path exists
            int is_dir = 0;
            ret = path_is_dir(p, &is_dir);
            if (ret) {
                // could not check if it is a directory, bail out
                break;
            }
            if (is_dir) {
                // path exists and is a directory
                if (rmdir(p->name.ptr) < 0) {
                    ret = errno;
                    break;
                }
            } else {
                // path exists and is not a directory
                ret = EEXIST;
                break;
            }
        } else {
        }
    } while (0);
    return ret;
}

int path_touch(Path* p) {
    int ret = 0;
    do {
        int exists = 0;
        ret = path_exists(p, &exists);
        if (ret) {
            // could not check existence, bail out
            break;
        }
        if (exists) {
            // path exists
            int is_file = 0;
            ret = path_is_file(p, &is_file);
            if (ret) {
                // could not check if it is a file, bail out
                break;
            }
            if (is_file) {
                // path exists and is a file
                if (utimes(p->name.ptr, 0) < 0) {
                    ret = errno;
                    break;
                }
            } else {
                // path exists and is not a file
                ret = EEXIST;
                break;
            }
        } else {
            // path does not exist
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
            dev_t dev = {0};
            if (mknod(p->name.ptr, S_IFREG | mode, dev) < 0) {
                ret = errno;
                break;
            }
        }
    } while (0);
    return ret;
}

int path_unlink(Path* p) {
    int ret = 0;
    do {
        if (unlink(p->name.ptr) < 0) {
            ret = errno;
            break;
        }
    } while (0);
    return ret;
}

int path_slurp(Path* p, Buffer* b) {
    int ret = 0;
    FILE* fp = 0;
    do {
        fp = fopen(p->name.ptr, "r");
        if (!fp) {
            ret = errno;
            break;
        }

        char tmp[BUFFER_SIZE];
        size_t nread = fread(tmp, 1, BUFFER_SIZE, fp);
        if (!nread) {
            break;
        }

        buffer_append_string(b, tmp, nread);
    } while (0);
    if (fp) {
        fclose(fp);
        fp = 0;
    }
    return ret;
}

static int write_to_file(Path* p, Slice s, const char* mode) {
    int ret = 0;
    FILE* fp = 0;
    do {
        fp = fopen(p->name.ptr, mode);
        if (!fp) {
            ret = errno;
            break;
        }
        size_t pos = 0;
        do {
            size_t nwritten = fwrite(s.ptr + pos, 1, s.len - pos, fp);
            if (!nwritten) {
                break;
            }
            pos += nwritten;
        } while (pos < s.len);
    } while (0);
    if (fp) {
        fclose(fp);
        fp = 0;
    }
    return ret;
}

int path_spew(Path* p, Slice s) {
    return write_to_file(p, s, "w");
}

int path_append(Path* p, Slice s) {
    return write_to_file(p, s, "a");
}

static int last_slash(Path* p) {
    int pos = p->name.len - 1;
    while (pos >= 0 && p->name.ptr[pos] != '/') {
        --pos;
    }
    if (pos < 0) {
        return -1;
    }
    return pos;
}

int path_parent(Path* p, Path* parent) {
    buffer_clear(&parent->name);
    int pos = last_slash(p);
    if (pos < 0) {
        return -1;
    }
    if (pos == 0) {
        // special cases:
        //   parent of "/foo" is "/"
        //   parent of "/" is "/"
        buffer_append_byte(&parent->name, '/');
    } else {
        buffer_append_string(&parent->name, p->name.ptr, pos);
    }
    buffer_null_terminate(&parent->name);
    --parent->name.len; // HACK ALARM
    return 0;
}

int path_sibling(Path* p, Path* sibling, Slice name) {
    buffer_clear(&sibling->name);
    int pos = last_slash(p);
    if (pos < 0) {
        return -1;
    }
    if (p->name.len <= 1) {
        return -1;
    }
    buffer_append_string(&sibling->name, p->name.ptr, pos + 1);
    buffer_append_slice(&sibling->name, name);
    buffer_null_terminate(&sibling->name);
    --sibling->name.len; // HACK ALARM
    return 0;
}

int path_child(Path* p, Path* child, Slice name) {
    buffer_clear(&child->name);
    int pos = last_slash(p);
    if (pos < 0) {
        return -1;
    }
    buffer_append_buffer(&child->name, &p->name);
    if (p->name.len > 1) {
        buffer_append_byte(&child->name, '/');
    }
    buffer_append_slice(&child->name, name);
    buffer_null_terminate(&child->name);
    --child->name.len; // HACK ALARM
    return 0;
}

int path_visit(Path* p, PathVisitor visit, void* arg) {
    int ret = 0;
    DIR* dir = 0;
    do {
       dir = opendir(p->name.ptr);
       if (!dir) {
           ret = errno;
           break;
       }
       while (1) {
           errno = 0;
           struct dirent* entry = readdir(dir);
           if (!entry) {
               ret = errno;
               break;
           }
           visit(p, entry, arg);
       }
    } while (0);
    if (dir) {
        closedir(dir);
        dir = 0;
    }
    return ret;
}

int path_skip_visit(const char* name) {
    if (name[0] == '.' && name[1] == '\0') return 1;
    if (name[0] == '.' && name[1] == '.' && name[2] == '\0') return 1;
    return 0;
}

const char* path_entry_type(uint8_t type) {
    switch (type) {
        case DT_REG:
            return "regular file";
        case DT_DIR:
            return "directory";
        case DT_LNK:
            return "symbolic link";
        case DT_FIFO:
            return "named pipe (FIFO)";
        case DT_SOCK:
            return "UNIX domain socket";
        case DT_CHR:
            return "character device";
        case DT_BLK:
            return "block device";
    }
    return "unknown";
}
