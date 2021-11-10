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

int path_touch(Path* p) {
    int ret = 0;
    FILE* fp = 0;
    do {
        int exists = 0;
        ret = path_exists(p, &exists);
        if (ret) {
            // could not check existence, bail out
            break;
        }
        if (!exists) {
            // it does not exist, create it
            // TODO: cheaper way to do this?
            fp = fopen(p->name.ptr, "w");
            break;
        }
        // it exists, update access and modification times to current
        if (utimes(p->name.ptr, 0) < 0) {
            ret = errno;
            break;
        }
    } while (0);
    if (fp) {
        fclose(fp);
        fp = 0;
    }
    return ret;
}

int path_remove(Path* p) {
    int ret = 0;
    do {
        if (remove(p->name.ptr) < 0) {
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
    if (pos <= 1) {
        return -1;
    }
    while (pos >= 0 && p->name.ptr[pos] != '/') {
        --pos;
    }
    if (pos < 0) {
        return -1;
    }
    return pos;
}

int path_parent(Path* p, Path* parent) {
    int pos = last_slash(p);
    if (pos < 0) {
        // TODO
        return -1;
    }
    buffer_append_string(&parent->name, p->name.ptr, pos);
    buffer_null_terminate(&parent->name);
    --parent->name.len; // HACK ALARM
    return 0;
}

int path_sibling(Path* p, Path* sibling, Slice name) {
    int pos = last_slash(p);
    if (pos < 0) {
        // TODO
        return -1;
    }
    buffer_append_string(&sibling->name, p->name.ptr, pos);
    buffer_append_byte(&sibling->name, '/');
    buffer_append_slice(&sibling->name, name);
    buffer_null_terminate(&sibling->name);
    --sibling->name.len; // HACK ALARM
    return 0;
}

int path_child(Path* p, Path* child, Slice name) {
    buffer_clear(&child->name);
    buffer_append_buffer(&child->name, &p->name);
    buffer_append_byte(&child->name, '/');
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

const char* path_entry_type(uint8_t type) {
    switch (type) {
        case DT_BLK:
            return "block device";
        case DT_CHR:
            return "character device";
        case DT_DIR:
            return "directory";
        case DT_FIFO:
            return "named pipe (FIFO)";
        case DT_LNK:
            return "symbolic link";
        case DT_REG:
            return "regular file";
        case DT_SOCK:
            return "UNIX domain socket";
    }
    return "unknown";
}
