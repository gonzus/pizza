#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <tap.h>
#include "path.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_path_mkdir(Path* tmp) {
    time_t seconds = time(0);
    struct tm* local = localtime(&seconds);
    char dir[1024];
    snprintf(dir, 512, "/tmp/pizza_test_path_%04d%02d%02d_%02d%02d%02d_%d",
            local->tm_year + 1900, local->tm_mon  + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec,
            getpid());
    int e = 0;
    path_from_string(tmp, dir, 0);

    path_exists(tmp, &e);
    ok(!e, "path [%s] does not exist initially", tmp->name.ptr);

    path_mkdir(tmp);
    path_exists(tmp, &e);
    ok(e, "path [%s] successfully created", tmp->name.ptr);
}

static void test_path_rmdir(Path* tmp) {
    int e = 0;

    path_exists(tmp, &e);
    ok(e, "path [%s] already exists", tmp->name.ptr);

    path_rmdir(tmp);
    path_exists(tmp, &e);
    ok(!e, "path [%s] successfully removed", tmp->name.ptr);
}

static void test_path_node(Path* tmp) {
    static struct {
        const char* name;
        int type;
    } data[] = {
        { "regular_file"          , S_IFREG  },
#if 0
        // cannot test these two, they require root permissions
        { "character_special_file", S_IFCHR  },
        { "block_special_file"    , S_IFBLK  },
#endif
        { "FIFO_named_pipe"       , S_IFIFO  },
        { "UNIX_domain_socket"    , S_IFSOCK },
    };

    for (uint32_t j = 0; j < ALEN(data); ++j) {
        int e = 0;
        char name[1024];
        snprintf(name, 1024, "%s/%s", tmp->name.ptr, data[j].name);
        Path p; path_from_string(&p, name, 0);

        path_exists(&p, &e);
        ok(!e, "path [%s] does not exist initially", name);

        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
        dev_t dev = 666;
        int m = mknod(name, data[j].type | mode, dev);
        ok(m == 0, "mknod() OK for path [%s]: %d %d", name, m, errno);
        if (m < 0) continue;

        path_exists(&p, &e);
        ok(e, "path [%s] successfully created", tmp->name.ptr);

        int u = unlink(name);
        ok(u == 0, "unlink() OK for path [%s]", name);
        if (u < 0) continue;

        path_exists(&p, &e);
        ok(!e, "path [%s] successfully removed", tmp->name.ptr);

        path_destroy(&p);
    }
}

static void test_path_symlink(Path* tmp) {
    int e = 0;

    Path ptgt; path_build(&ptgt);
    path_child(tmp, &ptgt, slice_from_string("regular_file.txt", 0));

    path_exists(&ptgt, &e);
    ok(!e, "path [%s] does not exist initially", ptgt.name.ptr);

    path_touch(&ptgt);
    path_exists(&ptgt, &e);
    ok(e, "path [%s] now exists", ptgt.name.ptr);

    Path psym; path_build(&psym);
    path_child(tmp, &psym, slice_from_string("symlink_to_regular_file.txt", 0));

    path_exists(&psym, &e);
    ok(!e, "path [%s] does not exist initially", psym.name.ptr);

    int r = symlink(ptgt.name.ptr, psym.name.ptr);
    ok(r == 0, "symlink() OK for [%s] => [%s]", psym.name.ptr, ptgt.name.ptr);
    path_exists(&psym, &e);
    ok(e, "path [%s] now exists", psym.name.ptr);

    Slice et = slice_from_string(ptgt.name.ptr, 0);
    Buffer b; buffer_build(&b);
    path_readlink(&psym, &b);
    Slice rt = buffer_slice(&b);
    ok(slice_equal(rt, et), "path [%s] points to [%.*s]", psym.name.ptr, rt.len, rt.ptr);
    buffer_destroy(&b);

    path_unlink(&psym);
    path_destroy(&psym);

    path_unlink(&ptgt);
    path_destroy(&ptgt);
}

static void test_path_touch(Path* tmp) {
    Path p; path_build(&p);
    path_child(tmp, &p, slice_from_string("path_touch.txt", 0));
    int e = 0;

    path_exists(&p, &e);
    ok(!e, "path [%s] does not exist before being touched", p.name.ptr);

    path_touch(&p);
    path_exists(&p, &e);
    ok(e, "path [%s] exists after being touched", p.name.ptr);

    path_unlink(&p);
    path_exists(&p, &e);
    ok(!e, "path [%s] does not exist anymore", p.name.ptr);

    path_destroy(&p);
}

static void test_path_spew_slurp_append(Path* tmp) {
    Path p; path_build(&p);
    path_child(tmp, &p, slice_from_string("path_slurp_spew_append.txt", 0));
    Buffer bs; buffer_build(&bs);
    Buffer ba; buffer_build(&ba);
    Buffer br; buffer_build(&br);
    Slice ts, ta, tr;
    int e = 0;

    path_exists(&p, &e);
    ok(!e, "path [%s] does not exist before being spewed to", p.name.ptr);

    Slice ss = slice_from_string("In a hole in the ground there lived a Hobbit.\n", 0);
    for (int j = 0; j < 100; ++j) {
        buffer_append_slice(&bs, ss);
    }
    ts = buffer_slice(&bs);
    path_spew(&p, ts);

    path_exists(&p, &e);
    ok(e, "path [%s] exists after being spewed to", p.name.ptr);

    buffer_clear(&br);
    path_slurp(&p, &br);
    tr = buffer_slice(&br);
    ok(slice_equal(ts, tr), "path [%s] has correct contents after being spewed to", p.name.ptr);

    Slice sa = slice_from_string("This was a Hobbit hole, and that means comfort.\n", 0);
    for (int j = 0; j < 100; ++j) {
        buffer_append_slice(&ba, sa);
    }
    ta = buffer_slice(&ba);
    path_append(&p, ta);

    path_exists(&p, &e);
    ok(e, "path [%s] still exists after being appended to", p.name.ptr);

    buffer_append_slice(&bs, ta);
    ts = buffer_slice(&bs);

    buffer_clear(&br);
    path_slurp(&p, &br);
    tr = buffer_slice(&br);
    ok(slice_equal(ts, tr), "path [%s] has correct contents after being appended to", p.name.ptr);

    buffer_clear(&bs);
    Slice sf = slice_from_string("This particular hobbit was called Bilbo Baggins.\n", 0);
    for (int j = 0; j < 100; ++j) {
        buffer_append_slice(&bs, sf);
    }
    ts = buffer_slice(&bs);
    path_spew(&p, ts);

    path_exists(&p, &e);
    ok(e, "path [%s] exists after being spewed to", p.name.ptr);

    buffer_clear(&br);
    path_slurp(&p, &br);
    tr = buffer_slice(&br);
    ok(slice_equal(ts, tr), "path [%s] has correct contents after being spewed to when it already existed", p.name.ptr);

    path_unlink(&p);

    buffer_destroy(&br);
    buffer_destroy(&ba);
    buffer_destroy(&bs);
    path_destroy(&p);
}

#define BAZ     "baz"
#define INVALID "*INVALID*"

static void test_path_family(void) {
    static struct {
        const char* name;
        int valid_parent;
        const char* parent;
        int valid_sibling;
        const char* sibling;
        int valid_child;
        const char* child;
    } data[] = {
        { "/foo/bar" , 1,  "/foo"  , 1 , "/foo/" BAZ , 1, "/foo/bar/" BAZ },
        { "/foo"     , 1,  "/"     , 1 , "/" BAZ     , 1, "/foo/" BAZ     },
        { "/"        , 1,  "/"     , 0 , INVALID     , 1, "/" BAZ         },
        { ""         , 0,  INVALID , 0 , INVALID     , 0, INVALID         },
        { "abc"      , 0,  INVALID , 0 , INVALID     , 0, INVALID         },
    };

    Slice baz = slice_from_string(BAZ, 0);
    for (uint32_t j = 0; j < ALEN(data); ++j) {
        Slice en = slice_from_string(data[j].name, 0);
        int err = 0;
        Path pn; path_from_slice(&pn, en);

        Path pp; path_build(&pp);
        err = path_parent(&pn, &pp);
        ok(data[j].valid_parent ? !err : err, "Got correct parent return [%d] for [%.*s]",
           err, en.len, en.ptr);
        if (!err) {
            Slice gp = buffer_slice(&pp.name);
            Slice ep = slice_from_string(data[j].parent, 0);
            ok(slice_equal(gp, ep), "Got parent right for [%.*s] => [%s]",
                    en.len, en.ptr, pp.name.ptr);
        }

        Path ps; path_build(&ps);
        err = path_sibling(&pn, &ps, baz);
        ok(data[j].valid_sibling ? !err : err, "Got correct sibling return [%d] for [%.*s]",
           err, en.len, en.ptr);
        if (!err) {
            Slice gs = buffer_slice(&ps.name);
            Slice es = slice_from_string(data[j].sibling, 0);
            ok(slice_equal(gs, es), "Got sibling right for [%.*s] => [%s]",
                    en.len, en.ptr, ps.name.ptr);
        }

        Path pc; path_build(&pc);
        err = path_child(&pn, &pc, baz);
        ok(data[j].valid_child ? !err : err, "Got correct child return [%d] for [%.*s]",
           err, en.len, en.ptr);
        if (!err) {
            Slice gc = buffer_slice(&pc.name);
            Slice ec = slice_from_string(data[j].child, 0);
            ok(slice_equal(gc, ec), "Got child right for [%.*s] => [%s]",
                    en.len, en.ptr, pc.name.ptr);
        }

        path_destroy(&pc);
        path_destroy(&ps);
        path_destroy(&pp);
        path_destroy(&pn);
    }
}

enum {
    VISIT_BLOCKDEV,
    VISIT_CHARDEV,
    VISIT_DIR,
    VISIT_FILE,
    VISIT_SYMLINK,
    VISIT_FIFO,
    VISIT_SOCKET,
    VISIT_LAST,
};

struct VisitMap {
    uint8_t d_type;
    int visit;
} VisitMap[] = {
  { DT_BLK , VISIT_BLOCKDEV },
  { DT_CHR , VISIT_CHARDEV  },
  { DT_DIR , VISIT_DIR      },
  { DT_REG , VISIT_FILE     },
  { DT_LNK , VISIT_SYMLINK  },
  { DT_FIFO, VISIT_FIFO     },
  { DT_SOCK, VISIT_SOCKET   },
};

typedef struct VisitStats {
    int visits[VISIT_LAST];
} VisitStats;

static int visitor(Path* p, struct dirent* entry, void* arg) {
    VisitStats* stats = (VisitStats*) arg;
    do {
        if (!entry) break;
#if 0
        printf("[%.*s] [%s] => [%llu] [%u] - [%s]\n",
               p->name.len, p->name.ptr, entry->d_name, entry->d_ino,
               (uint32_t) entry->d_type, path_entry_type(entry->d_type));
#endif
        for (uint32_t w = 0; w < VISIT_LAST; ++w) {
            if (VisitMap[w].d_type != entry->d_type) continue;

            ++stats->visits[VisitMap[w].visit];
            break;
        }
        if (entry->d_type != DT_DIR) break; // stop recursion unless dir
        if (path_skip_visit(entry->d_name)) break;

        Path child; path_build(&child);
        path_child(p, &child, slice_from_string(entry->d_name, 0));
        path_visit(&child, visitor, arg);
        path_destroy(&child);

    } while (0);
    return 0;
}

struct VisitInfo {
    int visit;
    int min;
};

static struct VisitShow {
    uint8_t type;
    const char* label;
} VisitShow[] = {
    { VISIT_BLOCKDEV, "blockdevs" },
    { VISIT_CHARDEV, "chardevs"  },
    { VISIT_DIR , "dirs"      },
    { VISIT_FILE, "files"     },
    { VISIT_SYMLINK, "symlinks"  },
    { VISIT_FIFO, "fifos"     },
    { VISIT_SOCKET, "sockets"   },
};

static void test_path_visit_dir(const char* dir, struct VisitInfo* info) {
    VisitStats stats = {0};
    Path p; path_from_string(&p, dir, 0);
    path_visit(&p, visitor, &stats);
    for (uint32_t what = 0; what < VISIT_LAST; ++what) {
        ok(stats.visits[what] >= info[what].min, "[%.*s]: found %4d >= %-2d %s",
           p.name.len, p.name.ptr, stats.visits[what], info[what].min, VisitShow[what].label);
    }
    path_destroy(&p);
}

static void test_path_visit() {
  struct VisitInfo vi_bin[] = {
    { VISIT_BLOCKDEV, 0 },
    { VISIT_CHARDEV , 0 },
    { VISIT_DIR     , 1 },
    { VISIT_FILE    , 1 },
    { VISIT_SYMLINK , 0 },
    { VISIT_FIFO    , 0 },
    { VISIT_SOCKET  , 0 },
  };
  struct VisitInfo vi_etc[] = {
    { VISIT_BLOCKDEV, 0 },
    { VISIT_CHARDEV , 0 },
    { VISIT_DIR     , 1 },
    { VISIT_FILE    , 1 },
    { VISIT_SYMLINK , 1 },
    { VISIT_FIFO    , 0 },
    { VISIT_SOCKET  , 0 },
  };
  struct VisitInfo vi_dev[] = {
    { VISIT_BLOCKDEV, 1 },
    { VISIT_CHARDEV , 1 },
    { VISIT_DIR     , 1 },
    { VISIT_FILE    , 0 },
    { VISIT_SYMLINK , 1 },
    { VISIT_FIFO    , 0 },
    { VISIT_SOCKET  , 0 },
  };
  struct VisitConfig {
    const char* path;
    struct VisitInfo* info;
  } config[] = {
    { "/dev", vi_dev },
    { "/bin", vi_bin },
    { "/etc", vi_etc },

  };
  for (unsigned j = 0; j < ALEN(config); ++j) {
    test_path_visit_dir(config[j].path, config[j].info);
  }
}

int main (int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    test_path_family();
    test_path_visit();

    Path tmp;
    test_path_mkdir(&tmp);

    test_path_node(&tmp);
    test_path_symlink(&tmp);
    test_path_touch(&tmp);
    test_path_spew_slurp_append(&tmp);

    test_path_rmdir(&tmp);
    path_destroy(&tmp);

    done_testing();
}
