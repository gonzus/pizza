#include <string.h>
#include <tap.h>
#include "path.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_path_types(void) {
    static struct {
        const char* name;
        int exists;
        char type;
    } data[] = {
        { "/gonzo-rules"  , 0, ' ' },
        { "/etc/passwd"   , 1, 'F' },
        { "/etc/X11"      , 1, 'D' },
        { "/etc/localtime", 1, 'S' },
    };

    for (int j = 0; j < ALEN(data); ++j) {
        Path p; path_from_string(&p, data[j].name, 0);
        int e = 0; path_exists(&p, &e);
        ok(e == data[j].exists, "path [%s] %s exist", data[j].name, data[j].exists ? "DOES" : "DOES NOT");
        if (!e) continue;

        int f = 0; path_is_file(&p, &f);
        ok(data[j].type == 'F' ? f : !f, "path [%s] %s a %s", data[j].name, data[j].type == 'F' ? "IS" : "IS NOT", "file");

        int d = 0; path_is_dir(&p, &d);
        ok(data[j].type == 'D' ? d : !d, "path [%s] %s a %s", data[j].name, data[j].type == 'D' ? "IS" : "IS NOT", "directory");

        int s = 0; path_is_symlink(&p, &s);
        ok(data[j].type == 'S' ? s : !s, "path [%s] %s a %s", data[j].name, data[j].type == 'S' ? "IS" : "IS NOT", "symlink");
        path_destroy(&p);
    }
}

static void test_path_readlink(void) {
    Slice s = slice_from_string("/etc/localtime", 0);
    Slice t = slice_from_string("/usr/share/zoneinfo/Europe/Amsterdam", 0);
    Path p; path_from_slice(&p, s);
    Buffer b; buffer_build(&b);
    path_readlink(&p, &b);
    ok(slice_equal(buffer_slice(&b), t), "path [%.*s] points to [%.*s]", s.len, s.ptr, t.len, t.ptr);
    buffer_destroy(&b);
    path_destroy(&p);
}

static void test_path_touch(void) {
    Slice s = slice_from_string("/tmp/path_touch.txt", 0);
    Path p; path_from_slice(&p, s);

    path_touch(&p);
    int e = 0; path_exists(&p, &e);
    ok(e, "path [%.*s] exists after being touched", s.len, s.ptr);

    path_remove(&p);
    path_destroy(&p);
}

static void test_path_slurp(void) {
    Slice s = slice_from_string("/etc/timezone", 0);
    Path p; path_from_slice(&p, s);

    Slice c = slice_from_string("Europe/Amsterdam\n", 0);

    Buffer b; buffer_build(&b);
    path_slurp(&p, &b);
    Slice t = buffer_slice(&b);

    ok(slice_equal(t, c), "path [%.*s] contains correct %u bytes", s.len, s.ptr, c.len);
    buffer_destroy(&b);
    path_destroy(&p);
}

static void test_path_spew(void) {
    Slice s = slice_from_string("/tmp/path_spew.txt", 0);
    Path p; path_from_slice(&p, s);

    Slice c = slice_from_string("Europe/Amsterdam\n", 0);
    path_spew(&p, c);

    Buffer b; buffer_build(&b);
    path_slurp(&p, &b);
    Slice t = buffer_slice(&b);
    path_remove(&p);

    ok(slice_equal(t, c), "path [%.*s] ended up with correct %u bytes", s.len, s.ptr, c.len);
    buffer_destroy(&b);
    path_destroy(&p);
}

static void test_path_append(void) {
    const int times = 5;

    Slice s = slice_from_string("/tmp/path_append.txt", 0);
    Path p; path_from_slice(&p, s);

    Slice c = slice_from_string("Europe/Amsterdam\n", 0);
    for (int j = 0; j < times; ++j) {
        if (j == 0)
            path_spew(&p, c);
        else
            path_append(&p, c);
    }

    Buffer b; buffer_build(&b);
    path_slurp(&p, &b);
    Slice t = buffer_slice(&b);
    path_remove(&p);

    Buffer d; buffer_build(&d);
    for (int j = 0; j < times; ++j) {
        buffer_append_slice(&d, c);
    }

    Slice f = buffer_slice(&d);
    ok(slice_equal(t, f), "path [%.*s] ended up with correct %u bytes", s.len, s.ptr, f.len);

    buffer_destroy(&d);
    buffer_destroy(&b);
    path_destroy(&p);
}

static int visitor(Path* p, struct dirent* entry, void* arg) {
    do {
        if (!entry) break;

#if 0
        printf("[%.*s] [%s] => [%lu] [%u] - [%s]\n",
               p->name.len, p->name.ptr, entry->d_name, entry->d_ino,
               (uint32_t) entry->d_type, entry_type(entry->d_type));
#endif
        if (entry->d_type != DT_DIR) break;

        const char* n = entry->d_name;
        if ((n[0] == '.' && n[1] == '\0') ||
            (n[0] == '.' && n[1] == '.' && n[2] == '\0')) break;

        Path child; path_build(&child);
        path_child(p, &child, slice_from_string(n, 0));
        path_visit(&child, visitor, arg);
        path_destroy(&child);

    } while (0);
    return 0;
}

static void test_path_parent(void) {
    const char* name = "/a/b/c";
    const char* expected = "/a/b";
    Path p; path_from_string(&p, name, 0);
    Path parent; path_build(&parent);
    path_parent(&p, &parent);
    ok(memcmp(expected, parent.name.ptr, strlen(expected)) == 0, "Got parent right: [%s] %u [%.*s]", name, parent.name.len, parent.name.len, parent.name.ptr);
    path_destroy(&p);
}

static void test_path_sibling(void) {
    const char* name = "/a/b/c";
    const char* expected = "/a/b/y";
    Path p; path_from_string(&p, name, 0);
    Path sibling; path_build(&sibling);
    path_sibling(&p, &sibling, slice_from_string("y", 0));
    ok(memcmp(expected, sibling.name.ptr, strlen(expected)) == 0, "Got sibling right: [%s] %u [%.*s]", name, sibling.name.len, sibling.name.len, sibling.name.ptr);
    path_destroy(&p);
}

static void test_path_child(void) {
    const char* name = "/a/b/c";
    const char* expected = "/a/b/c/x";
    Path p; path_from_string(&p, name, 0);
    Path child; path_build(&child);
    path_child(&p, &child, slice_from_string("x", 0));
    ok(memcmp(expected, child.name.ptr, strlen(expected)) == 0, "Got child right: [%s] %u [%.*s]", name, child.name.len, child.name.len, child.name.ptr);
    path_destroy(&p);
}

static void test_path_visit(void) {
    // Slice s = slice_from_string("/etc/systemd", 0);
    Path p; path_from_string(&p, "/dev", 0);
    path_visit(&p, visitor, 0);
    path_destroy(&p);
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_path_types();
    test_path_readlink();
    test_path_touch();
    test_path_slurp();
    test_path_spew();
    test_path_append();
    test_path_visit();
    test_path_parent();
    test_path_sibling();
    test_path_child();

    done_testing();
}
