#ifndef PATH_H_
#define PATH_H_

/*
 * Path -- routines to deal with paths / files.
 * Inspired on Perl's https://metacpan.org/pod/Path::Tiny
 */

#include "buffer.h"

typedef struct Path {
    Buffer name;
} Path;

// Path constructor from a file name given as a string (const char*).
// If len < 0, use null terminator, otherwise copy len bytes.
void path_from_string(Path* p, const char* str, int len);

// Path constructor from a file name given as a Slice.
void path_from_slice(Path* p, Slice s);

// Destroy a Path.
void path_destroy(Path* p);

int path_exists(Path* p, int* exists);
int path_is_file(Path* p, int* is_file);
int path_is_dir(Path* p, int* is_dir);
int path_is_symlink(Path* p, int* is_symlink);
int path_readlink(Path* p, Buffer* b);

// If path does not exist, create it (empty).
// If it exists, update modification and access times.
// Return 0 for success, non-zero for error conditions.
int path_touch(Path* p);

// Make sure path does not exist in fs anymore.
// Return 0 for success, non-zero for error conditions.
int path_remove(Path* p);

// Append to a Buffer the contents of file given by p.
// Return 0 for success, non-zero for error conditions.
int path_slurp(Path* p, Buffer* b);

// Write the contents of a Slice to file given by p.
// Return 0 for success, non-zero for error conditions.
// File will be created / overwritten.
int path_spew(Path* p, Slice s);

// Append the contents of a Slice to file given by p.
// Return 0 for success, non-zero for error conditions.
// File will be created / appended to.
int path_append(Path* p, Slice s);

#if 0
int path_child(Path* p, Path* c);
#endif

#endif
