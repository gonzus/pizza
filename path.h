#ifndef PATH_H_
#define PATH_H_

/*
 * Path -- routines to deal with paths / files.
 * Inspired on Perl's https://metacpan.org/pod/Path::Tiny
 */

#include <dirent.h>
#include "buffer.h"

typedef struct Path {
    Buffer name;
} Path;

typedef int (PathVisitor)(Path* p, struct dirent* entry, void* arg);

// Build an empty path.
void path_build(Path* p);

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

// If p = /a/b/c, parent => /a/b
int path_parent(Path* p, Path* parent);

// If p = /a/b/c and name = y, sibling => /a/b/y
int path_sibling(Path* p, Path* sibling, Slice name);

// If p = /a/b/c and name = x, child => /a/b/c/x
int path_child(Path* p, Path* child, Slice name);

int path_visit(Path* p, PathVisitor visit, void* arg);

// Get a string describing the entry type.
// Type is field d_type from struct dirent.
const char* path_entry_type(uint8_t type);

#endif
