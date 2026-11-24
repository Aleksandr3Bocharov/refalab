// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-23
// https://github.com/Aleksandr3Bocharov/refalab

//////////////////////////////////////////////////////////////////////////////
// bootstrap.c
// The first-stage builder of the RefalAB system: it needs only clang, no make,
// no shell scripts. Build and run it with one command:
//     POSIX:   clang -DPOSIX bootstrap.c -o bootstrap && ./bootstrap
//     Windows: clang bootstrap.c -o bootstrap.exe && bootstrap.exe
// As everywhere in RefalAB, the directive POSIX selects the POSIX environment;
// without it the Windows environment is assumed.
// Usage:
//     bootstrap            - build the release and the debug variants
//     bootstrap release    - build the release variant only
//     bootstrap debug      - build the debug variant only (-Dmdebug)
//     bootstrap clean      - remove build/, bin/ and lib/
// Result (the same layout as the Makefile build):
//     bin/refalabc, bin/refalabc_dbg                    - the compilers
//     lib/librefalab.a, lib/librefalab_dbg.a            - the libraries
//     lib/obj/refalab_initiator[_dbg].o, lib/obj/refalab_debugger[_dbg].o
// The intermediate objects are in build/<variant>/{comp,inter}.
// Like the interpreter modules (see xfileio.c), it uses <dirent.h> and mkdir,
// which clang provides on both platforms. Files and directories are handled
// by C code, not by shell commands (mkdir -p, cp, rm do not exist in cmd.exe).
//////////////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if !defined(POSIX)
#include <direct.h>
#define popen _popen
#define pclose _pclose
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
#define LSTAT(path, st) stat(path, st) // no symbolic links to care about on Windows
#define EXE ".exe"                      // the suffix of the executables
#define PLATFORM_CFLAGS ""              // no -DPOSIX on Windows
#define PLATFORM_LFLAGS "-pthread -static " // the linker flags of the compiler (as in src/comp/Makefile)
#define PLATFORM_SRC "xplatform_win"    // the platform module built as xplatform.o
#define OTHER_PLATFORM_SRC "xplatform_posix"
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#define LSTAT(path, st) lstat(path, st) // a symbolic link is removed, never followed
#define EXE ""
#define PLATFORM_CFLAGS " -DPOSIX"
#define PLATFORM_LFLAGS ""
#define PLATFORM_SRC "xplatform_posix"
#define OTHER_PLATFORM_SRC "xplatform_win"
#endif

#define CMD_SIZE 8192
#define MAX_FILES 256

// The build variant: release or debug
typedef struct
{
    const char *name;    // "release" / "debug"
    const char *suffix;  // "" / "_dbg"
    const char *cflags;  // flags of the C compiler (the platform flags are added)
    const char *llflags; // flags of clang for the LLVM IR of a RefalAB module
} variant_t;

static const variant_t variants[] = {
    {"release", "", "-pipe -Wall -O2", "-O2 -Wno-override-module"},
    {"debug", "_dbg", "-pipe -Wall -O0 -g -Dmdebug", "-O0 -g -Wno-override-module"}};

// The compiler version defines, the same as in src/comp/Makefile
static char clang_defs[512];

static void fail(const char *what, const char *path)
{
    fprintf(stderr, "bootstrap: %s: %s\n", what, path);
    exit(1);
}

// Print and execute a command; stop the build if it fails.
// On Windows the command goes to cmd.exe, which takes '/' for an option
// delimiter, so the slashes of the paths are replaced with backslashes.
static void run(const char *command)
{
    char cmd[CMD_SIZE];
    strncpy(cmd, command, sizeof cmd - 1);
    cmd[sizeof cmd - 1] = '\0';
#if !defined(POSIX)
    for (char *p = cmd; *p != '\0'; p++)
        if (*p == '/')
            *p = '\\';
#endif
    printf("%s\n", cmd);
    fflush(stdout);
    if (system(cmd) != 0)
        fail("command failed", cmd);
}

// Copy the token (up to a white space) from s to dst
static void copy_token(char *dst, size_t size, const char *s)
{
    size_t n = 0;
    while (s[n] != '\0' && s[n] != ' ' && s[n] != '\t' && s[n] != '\r' && s[n] != '\n' && n + 1 < size)
    {
        dst[n] = s[n];
        n++;
    }
    dst[n] = '\0';
}

// Get the version, the target and the thread model from the output of "clang -v"
static void detect_clang(void)
{
    char line[512], version[128] = "", target[128] = "", thread_model[128] = "";
    FILE *p = popen("clang -v 2>&1", "r");
    if (p == NULL)
        fail("cannot run", "clang -v");
    while (fgets(line, sizeof line, p) != NULL)
    {
        const char *s = strstr(line, "clang version ");
        if (s != NULL && version[0] == '\0')
            copy_token(version, sizeof version, s + strlen("clang version "));
        else if (strncmp(line, "Target: ", 8) == 0)
            copy_token(target, sizeof target, line + 8);
        else if (strncmp(line, "Thread model: ", 14) == 0)
            copy_token(thread_model, sizeof thread_model, line + 14);
    }
    pclose(p);
    if (version[0] == '\0')
        fail("clang is not found", "clang -v");
    // -DNAME="\"value\"" is understood by sh and by cmd.exe alike
    snprintf(clang_defs, sizeof clang_defs,
             "-DCLANG_VERSION=\"\\\"%s\\\"\" -DCLANG_TARGET_TRIPLE=\"\\\"%s\\\"\" -DCLANG_THREAD_MODEL=\"\\\"%s\\\"\"",
             version, target, thread_model);
}

static int cmp_names(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

// Collect the names of the files of a directory with the given extension (sorted)
static int list_files(const char *dir, const char *ext, char *names[])
{
    int n = 0;
    size_t el = strlen(ext);
    DIR *d = opendir(dir);
    if (d == NULL)
        fail("cannot open the directory (run it from the root of RefalAB)", dir);
    for (struct dirent *e = readdir(d); e != NULL; e = readdir(d))
    {
        size_t l = strlen(e->d_name);
        if (l > el && strcmp(e->d_name + l - el, ext) == 0 && n < MAX_FILES)
            names[n++] = strdup(e->d_name);
    }
    closedir(d);
    qsort(names, (size_t)n, sizeof(char *), cmp_names);
    return n;
}

static void free_names(char *names[], int n)
{
    for (int i = 0; i < n; i++)
        free(names[i]);
}

static time_t mtime(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 ? st.st_mtime : 0;
}

// The newest modification time of the files with the extension ext in a directory
static time_t newest(const char *dir, const char *ext)
{
    char *names[MAX_FILES];
    char path[512];
    time_t t = 0;
    int n = list_files(dir, ext, names);
    for (int i = 0; i < n; i++)
    {
        snprintf(path, sizeof path, "%s/%s", dir, names[i]);
        time_t m = mtime(path);
        if (m > t)
            t = m;
    }
    free_names(names, n);
    return t;
}

// An object must be rebuilt if it is absent or older than the given time
static int stale(const char *obj, time_t dep)
{
    time_t t = mtime(obj);
    return t == 0 || t < dep;
}

// Create a directory with all its parents (like mkdir -p)
static void make_dir(const char *path)
{
    char buf[512];
    strncpy(buf, path, sizeof buf - 1);
    buf[sizeof buf - 1] = '\0';
    for (char *p = buf + 1; ; p++)
    {
        if (*p == '/' || *p == '\0')
        {
            char c = *p;
            *p = '\0';
            if (MKDIR(buf) != 0 && errno != EEXIST)
                fail("cannot create the directory", buf);
            *p = c;
            if (c == '\0')
                break;
        }
    }
}

// Copy a file (like cp)
static void copy_file(const char *from, const char *to)
{
    FILE *in = fopen(from, "rb");
    if (in == NULL)
        fail("cannot open the file", from);
    FILE *out = fopen(to, "wb");
    if (out == NULL)
        fail("cannot create the file", to);
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, n, out) != n)
            fail("cannot write the file", to);
    fclose(in);
    fclose(out);
}

// Remove a file or a directory with its contents (like rm -rf); a missing path is not an error
static void remove_tree(const char *path)
{
    struct stat st;
    if (LSTAT(path, &st) != 0)
        return;
    if (S_ISDIR(st.st_mode))
    {
        DIR *d = opendir(path);
        if (d == NULL)
            fail("cannot open the directory", path);
        for (struct dirent *e = readdir(d); e != NULL; e = readdir(d))
        {
            char sub[1024];
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
                continue;
            snprintf(sub, sizeof sub, "%s/%s", path, e->d_name);
            remove_tree(sub);
        }
        closedir(d);
        if (RMDIR(path) != 0)
            fail("cannot remove the directory", path);
    }
    else if (remove(path) != 0)
        fail("cannot remove the file", path);
}

// Build the C compiler of RefalAB: bin/refalabc[_dbg]
static void build_comp(const variant_t *v)
{
    char *names[MAX_FILES];
    char objdir[128], cmd[CMD_SIZE], obj[512], src[512], link[CMD_SIZE];
    time_t hdr = newest("src/comp", ".h");
    int n = list_files("src/comp", ".c", names);

    printf("=== Building compiler (%s) ===\n", v->name);
    snprintf(objdir, sizeof objdir, "build/%s/comp", v->name);
    make_dir(objdir);
    make_dir("bin");
    snprintf(link, sizeof link, "clang %s-o bin/refalabc%s" EXE, PLATFORM_LFLAGS, v->suffix);
    for (int i = 0; i < n; i++)
    {
        names[i][strlen(names[i]) - 2] = '\0'; // drop ".c"
        snprintf(src, sizeof src, "src/comp/%s.c", names[i]);
        snprintf(obj, sizeof obj, "%s/%s.o", objdir, names[i]);
        time_t dep = mtime(src) > hdr ? mtime(src) : hdr;
        if (stale(obj, dep))
        {
            snprintf(cmd, sizeof cmd, "clang %s" PLATFORM_CFLAGS " %s -c %s -o %s", v->cflags, clang_defs, src, obj);
            run(cmd);
        }
        strncat(link, " ", sizeof link - strlen(link) - 1);
        strncat(link, obj, sizeof link - strlen(link) - 1);
    }
    run(link);
    free_names(names, n);
}

// Build the interpreter library and the standard initiator/debugger objects
static void build_inter(const variant_t *v)
{
    char *cnames[MAX_FILES], *rnames[MAX_FILES];
    char objdir[128], tmpdir[128], cmd[CMD_SIZE], obj[512], src[512], tmp[512], ar[CMD_SIZE];
    const char *compiler = "bin/refalabc" EXE; // the release compiler, as in the Makefile
    time_t hdr = newest("src/inter", ".h");
    int nc = list_files("src/inter", ".c", cnames);
    int nr = list_files("src/inter", ".ref", rnames);

    printf("=== Building interpreter (%s) ===\n", v->name);
    snprintf(objdir, sizeof objdir, "build/%s/inter", v->name);
    snprintf(tmpdir, sizeof tmpdir, "build/%s/tmp", v->name);
    make_dir(objdir);
    make_dir(tmpdir);
    make_dir("lib/obj");
    snprintf(ar, sizeof ar, "ar rcs lib/librefalab%s.a", v->suffix);

    // C modules: the initiator and the debugger go to lib/obj, the rest to the library
    for (int i = 0; i < nc; i++)
    {
        int standalone = strcmp(cnames[i], "initiator.c") == 0 || strcmp(cnames[i], "debugger.c") == 0;
        cnames[i][strlen(cnames[i]) - 2] = '\0';
        snprintf(src, sizeof src, "src/inter/%s.c", cnames[i]);
        if (standalone)
            snprintf(obj, sizeof obj, "lib/obj/refalab_%s%s.o", cnames[i], v->suffix);
        else
            snprintf(obj, sizeof obj, "%s/%s.o", objdir, cnames[i]);
        time_t dep = mtime(src) > hdr ? mtime(src) : hdr;
        if (stale(obj, dep))
        {
            snprintf(cmd, sizeof cmd, "clang %s" PLATFORM_CFLAGS " -c %s -o %s", v->cflags, src, obj);
            run(cmd);
        }
        if (!standalone)
        {
            strncat(ar, " ", sizeof ar - strlen(ar) - 1);
            strncat(ar, obj, sizeof ar - strlen(ar) - 1);
        }
    }

    // RefalAB modules; the module of this platform is built as xplatform.o, the other one is skipped
    time_t cmp_time = mtime(compiler);
    for (int i = 0; i < nr; i++)
    {
        const char *objname;
        if (strcmp(rnames[i], OTHER_PLATFORM_SRC ".ref") == 0)
            continue;
        rnames[i][strlen(rnames[i]) - 4] = '\0'; // drop ".ref"
        objname = strcmp(rnames[i], PLATFORM_SRC) == 0 ? "xplatform" : rnames[i];
        snprintf(obj, sizeof obj, "%s/%s.o", objdir, objname);
        snprintf(src, sizeof src, "src/inter/%s.ref", rnames[i]);
        time_t dep = mtime(src) > cmp_time ? mtime(src) : cmp_time;
        if (stale(obj, dep))
        {
            // The compiler writes the .ll/.lst files next to the source: work in a temporary copy
            snprintf(tmp, sizeof tmp, "%s/%s.ref", tmpdir, rnames[i]);
            copy_file(src, tmp);
            snprintf(cmd, sizeof cmd, "%s %s/%s -l,\"-o %s %s\" > build/%s/%s.log", compiler, tmpdir, rnames[i], obj,
                     v->llflags, v->name, rnames[i]);
            run(cmd);
        }
        strncat(ar, " ", sizeof ar - strlen(ar) - 1);
        strncat(ar, obj, sizeof ar - strlen(ar) - 1);
    }
    // The library is rebuilt from scratch so that removed modules disappear from it
    snprintf(cmd, sizeof cmd, "lib/librefalab%s.a", v->suffix);
    remove_tree(cmd);
    run(ar);
    free_names(cnames, nc);
    free_names(rnames, nr);
}

int main(int argc, char *argv[])
{
    int first = 0, last = 1;
    if (argc > 1)
    {
        if (strcmp(argv[1], "clean") == 0)
        {
            printf("=== Cleaning build artifacts ===\n");
            remove_tree("build");
            remove_tree("bin");
            remove_tree("lib");
            return 0;
        }
        else if (strcmp(argv[1], "release") == 0)
            last = 0;
        else if (strcmp(argv[1], "debug") == 0)
            first = 1;
        else
        {
            fprintf(stderr, "usage: %s [release|debug|clean]\n", argv[0]);
            return 2;
        }
    }
    detect_clang();
    for (int i = first; i <= last; i++)
        build_comp(&variants[i]);
    if (first == 1)
        build_comp(&variants[0]); // the debug-only build still needs the release compiler for the .ref modules
    for (int i = first; i <= last; i++)
        build_inter(&variants[i]);
    printf("=======================================================\n");
    printf(" RefalAB bootstrap build successfully completed!\n");
    printf("=======================================================\n");
    return 0;
}
