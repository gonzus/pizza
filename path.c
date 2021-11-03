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

void path_from_string(Path* p, const char* str, int len) {
    buffer_build(&p->name);
    buffer_append_string(&p->name, str, len);
    buffer_null_terminate(&p->name);
}

void path_from_slice(Path* p, Slice s) {
    buffer_build(&p->name);
    buffer_append_slice(&p->name, s);
    buffer_null_terminate(&p->name);
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

int path_is_file(Path* p, int* is_file) {
    *is_file = 0;
    int ret = 0;
    do {
        struct stat sb;
        if (lstat(p->name.ptr, &sb) < 0) {
            ret = errno;
            break;
        }
        *is_file = S_ISREG(sb.st_mode);
    } while (0);
    return ret;
}

int path_is_dir(Path* p, int* is_dir) {
    *is_dir = 0;
    int ret = 0;
    do {
        struct stat sb;
        if (lstat(p->name.ptr, &sb) < 0) {
            ret = errno;
            break;
        }
        *is_dir = S_ISDIR(sb.st_mode);
    } while (0);
    return ret;
}

int path_is_symlink(Path* p, int* is_symlink) {
    *is_symlink = 0;
    int ret = 0;
    do {
        struct stat sb;
        if (lstat(p->name.ptr, &sb) < 0) {
            ret = errno;
            break;
        }
        *is_symlink = S_ISLNK(sb.st_mode);
    } while (0);
    return ret;
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
