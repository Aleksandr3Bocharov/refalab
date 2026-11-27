// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-23
// https://github.com/Aleksandr3Bocharov/refalab

//////////////////////////////////////////////////////////////////////////////
// bootstrap.c
// The first-stage builder of the RefalAB system: it needs only clang
// and no other tools or scripts. Build and run it with one command:
//     POSIX:   clang -DPOSIX bootstrap.c -o bootstrap && ./bootstrap
//     Windows: clang bootstrap.c -o bootstrap.exe && bootstrap.exe
// As everywhere in RefalAB, the directive POSIX selects the POSIX environment;
// without it the Windows environment is assumed.
// Usage:
//     bootstrap            - build the release and the debug variants
//     bootstrap release    - build the release variant only
//     bootstrap debug      - build the debug variant only (-Dmdebug)
//     bootstrap clean      - remove build/, bin/ and lib/ and remove bin/ from PATH
// After a successful build it offers to add the directory bin to the PATH of
// the user; "clean" removes it from PATH without asking.
//     Windows: the user variable Path in the registry (HKCU\Environment), the added directory
//              is remembered in HKCU\Software\RefalAB (BinDir) to remove the old one later;
//     POSIX:   a block between the lines "# >>> refalab >>>" and "# <<< refalab <<<"
//              in the startup file of the shell (~/.zshrc, ~/.bashrc or ~/.profile).
// Result:
//     bin/refalabc, bin/refalabc_dbg                    - the compilers
//     lib/librefalab.a, lib/librefalab_dbg.a            - the libraries
//     lib/obj/refalab_initiator[_dbg].o, lib/obj/refalab_debugger[_dbg].o
// The intermediate objects are in build/<variant>/{comp,inter}.
// Like the interpreter modules (see xfileio.c), it uses <dirent.h> and mkdir,
// which clang provides on both platforms. Files and directories are handled
// by C code, not by shell commands (mkdir -p, cp, rm do not exist in cmd.exe).
//////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if !defined(POSIX)
#include <direct.h>
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define MKDIR(path) _mkdir(path)
#define GETCWD(buf, size) _getcwd(buf, size)
#define RMDIR(path) _rmdir(path)
#define LSTAT(path, st) stat(path, st) // no symbolic links to care about on Windows
#define EXE ".exe"                      // the suffix of the executables
#define PLATFORM_CFLAGS ""              // no -DPOSIX on Windows
#define PLATFORM_LFLAGS "-pthread -static " // the linker flags of the compiler
#define PLATFORM_SRC "xplatform_win"    // the platform module built as xplatform.o
#define OTHER_PLATFORM_SRC "xplatform_posix"
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define GETCWD(buf, size) getcwd(buf, size)
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

// The defines with the version, the target and the thread model of clang
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
    while (s[n] != '\0' && !isspace((unsigned char)s[n]) && n + 1 < size)
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
    char objdir[128], cmd[CMD_SIZE], obj[512], src[512], exe[128], link[CMD_SIZE];
    time_t hdr = newest("src/comp", ".h"), newest_obj = 0;
    int rebuilt = 0; // some object was compiled by this run
    int n = list_files("src/comp", ".c", names);

    printf("=== Building compiler (%s) ===\n", v->name);
    snprintf(objdir, sizeof objdir, "build/%s/comp", v->name);
    make_dir(objdir);
    make_dir("bin");
    snprintf(exe, sizeof exe, "bin/refalabc%s" EXE, v->suffix);
    snprintf(link, sizeof link, "clang %s-o %s", PLATFORM_LFLAGS, exe);
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
            rebuilt = 1;
        }
        if (mtime(obj) > newest_obj)
            newest_obj = mtime(obj);
        strncat(link, " ", sizeof link - strlen(link) - 1);
        strncat(link, obj, sizeof link - strlen(link) - 1);
    }
    // Link only if an object was compiled by this run or is newer than the compiler (or it is absent).
    // The compiler must not be touched otherwise: the RefalAB modules are rebuilt when it is newer.
    if (rebuilt || stale(exe, newest_obj))
        run(link);
    free_names(names, n);
}

// Compile one RefalAB module src/inter/<name>.ref into the object file obj.
// This is where the RefalAB modules are compiled:
//   1. the module is copied to build/<variant>/tmp, because the compiler writes
//      its .ll and .lst files next to the source and the sources stay clean;
//   2. the release compiler bin/refalabc translates the copy into LLVM IR (.ll);
//   3. the option -l," ..." makes the compiler run clang on the .ll with these
//      arguments: -o <obj> and the flags of the variant, which gives the object;
//   4. the output of the compiler goes to build/<variant>/<name>.log.
static void compile_ref(const variant_t *v, const char *name, const char *obj)
{
    char src[512], tmp[512], cmd[CMD_SIZE];
    snprintf(src, sizeof src, "src/inter/%s.ref", name);
    snprintf(tmp, sizeof tmp, "build/%s/tmp/%s.ref", v->name, name);
    copy_file(src, tmp);
    snprintf(cmd, sizeof cmd, "bin/refalabc" EXE " build/%s/tmp/%s -l,\"-o %s %s\" > build/%s/%s.log", v->name, name,
             obj, v->llflags, v->name, name);
    run(cmd);
}

// Build the interpreter library and the standard initiator/debugger objects
static void build_inter(const variant_t *v)
{
    char *cnames[MAX_FILES], *rnames[MAX_FILES];
    char objdir[128], cmd[CMD_SIZE], obj[512], src[512], lib[128], ar[CMD_SIZE];
    time_t hdr = newest("src/inter", ".h"), newest_obj = 0;
    int rebuilt = 0; // some object of the library was compiled by this run
    int nc = list_files("src/inter", ".c", cnames);
    int nr = list_files("src/inter", ".ref", rnames);

    printf("=== Building interpreter (%s) ===\n", v->name);
    snprintf(objdir, sizeof objdir, "build/%s/inter", v->name);
    make_dir(objdir);
    snprintf(src, sizeof src, "build/%s/tmp", v->name); // the temporary copies of the .ref modules
    make_dir(src);
    make_dir("lib/obj");
    snprintf(lib, sizeof lib, "lib/librefalab%s.a", v->suffix);
    snprintf(ar, sizeof ar, "ar rcs %s", lib);

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
            if (!standalone)
                rebuilt = 1;
        }
        if (!standalone)
        {
            if (mtime(obj) > newest_obj)
                newest_obj = mtime(obj);
            strncat(ar, " ", sizeof ar - strlen(ar) - 1);
            strncat(ar, obj, sizeof ar - strlen(ar) - 1);
        }
    }

    // RefalAB modules (see compile_ref); the module of this platform is built as xplatform.o,
    // the other one is skipped. A module is rebuilt when its source or the compiler is newer.
    time_t cmp_time = mtime("bin/refalabc" EXE);
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
            compile_ref(v, rnames[i], obj);
            rebuilt = 1;
        }
        if (mtime(obj) > newest_obj)
            newest_obj = mtime(obj);
        strncat(ar, " ", sizeof ar - strlen(ar) - 1);
        strncat(ar, obj, sizeof ar - strlen(ar) - 1);
    }
    // The library is rebuilt from scratch, only if some object is newer than it (or it is absent),
    // so that the removed modules disappear from it
    if (rebuilt || stale(lib, newest_obj))
    {
        remove_tree(lib);
        run(ar);
    }
    free_names(cnames, nc);
    free_names(rnames, nr);
}

//////////////////////////////////////////////////////////////////////////////
// The user PATH
//////////////////////////////////////////////////////////////////////////////

enum
{
    PATH_CHECK, // is the directory in PATH?
    PATH_ADD,   // add the directory to PATH
    PATH_REMOVE // remove the directory from PATH
};

// The absolute path of the directory bin
static void bin_dir(char *dir, size_t size)
{
    char cwd[512];
    if (GETCWD(cwd, sizeof cwd) == NULL)
        fail("cannot get the current directory", ".");
    snprintf(dir, size, "%s/bin", cwd);
#if !defined(POSIX)
    for (char *p = dir; *p != '\0'; p++)
        if (*p == '/')
            *p = '\\';
#endif
}

#if !defined(POSIX)

// Windows: the user variable Path in the registry (HKCU\Environment). The directory that was added
// to Path is remembered in HKCU\Software\RefalAB, the value BinDir: it is what lets the old directory
// (the project was moved) be found and removed. The type of Path (REG_SZ or REG_EXPAND_SZ) is kept.

#define APP_KEY "Software\\RefalAB"

// The length of a path without the trailing slashes
static size_t trimmed_length(const char *p, size_t l)
{
    while (l > 0 && (p[l - 1] == '\\' || p[l - 1] == '/'))
        l--;
    return l;
}

// Copy the entries of the list old (separated by ';') to out, dropping the empty entries and
// the entry drop (a case-insensitive comparison, the trailing slashes are ignored).
// out may be NULL. Returns the number of the dropped entries.
static int path_rewrite(const char *old, const char *drop, char *out)
{
    int dropped = 0;
    size_t dl = drop != NULL ? trimmed_length(drop, strlen(drop)) : 0;
    if (out != NULL)
        out[0] = '\0';
    for (const char *p = old; *p != '\0';)
    {
        const char *end = strchr(p, ';');
        size_t l = end != NULL ? (size_t)(end - p) : strlen(p);
        size_t k = trimmed_length(p, l);
        if (l > 0)
        {
            if (drop != NULL && dl > 0 && k == dl && _strnicmp(p, drop, k) == 0)
                dropped++;
            else if (out != NULL)
            {
                if (out[0] != '\0')
                    strcat(out, ";");
                strncat(out, p, l);
            }
        }
        p += l;
        if (*p == ';')
            p++;
    }
    return dropped;
}

// Read a string value of a registry key into a new string (empty if there is no such value)
static char *reg_read(HKEY key, const char *name, DWORD *type)
{
    DWORD size = 0, t = REG_SZ;
    if (RegQueryValueExA(key, name, NULL, &t, NULL, &size) != ERROR_SUCCESS)
    {
        size = 0;
        t = REG_SZ;
    }
    char *value = calloc((size_t)size + 2, 1);
    if (value == NULL)
        fail("out of memory", name);
    if (size > 0)
    {
        DWORD got = size;
        RegQueryValueExA(key, name, NULL, NULL, (LPBYTE)value, &got);
    }
    if (type != NULL)
        *type = t;
    return value;
}

static void reg_write(HKEY key, const char *name, DWORD type, const char *value)
{
    if (RegSetValueExA(key, name, 0, type, (const BYTE *)value, (DWORD)strlen(value) + 1) != ERROR_SUCCESS)
        fprintf(stderr, "bootstrap: cannot write %s to the registry\n", name);
}

// Let the running programs (Explorer, new terminals) see the change of the environment
static void broadcast_change(void)
{
    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

// PATH_CHECK:  returns 1 if dir is in Path and it is the directory remembered in BinDir.
// PATH_ADD:    removes the remembered directory from Path (if it is another one), adds dir, remembers dir.
// PATH_REMOVE: removes the remembered directory and dir from Path, forgets the remembered one;
//              returns 1 if something was removed from Path.
static int user_path(const char *dir, int op)
{
    HKEY env, app;
    DWORD type;
    int result = 0, changed = 0;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &env) != ERROR_SUCCESS)
    {
        fprintf(stderr, "bootstrap: cannot open the user environment in the registry\n");
        return 0;
    }
    char *saved = strdup(""); // the directory added by the previous run
    if (RegOpenKeyExA(HKEY_CURRENT_USER, APP_KEY, 0, KEY_QUERY_VALUE, &app) == ERROR_SUCCESS)
    {
        free(saved);
        saved = reg_read(app, "BinDir", NULL);
        RegCloseKey(app);
    }
    char *old = reg_read(env, "Path", &type);
    if (type != REG_SZ && type != REG_EXPAND_SZ)
        type = REG_EXPAND_SZ;
    size_t cap = strlen(old) + strlen(saved) + strlen(dir) + 8;
    char *work = calloc(cap, 1), *next = calloc(cap, 1);
    if (saved == NULL || work == NULL || next == NULL)
        fail("out of memory", "PATH");

    if (op == PATH_CHECK)
    {
        size_t dl = trimmed_length(dir, strlen(dir));
        result = path_rewrite(old, dir, NULL) > 0 && trimmed_length(saved, strlen(saved)) == dl &&
                 _strnicmp(saved, dir, dl) == 0;
    }
    else
    {
        // 1. Drop the remembered directory (it may be the old place of the project)
        path_rewrite(old, saved, work);
        if (op == PATH_REMOVE)
        {
            // 2. Drop the current one, even if it was not remembered
            result = path_rewrite(old, saved, NULL) > 0;
            strcpy(next, work);
            if (path_rewrite(next, dir, work) > 0)
                result = 1;
        }
        else
        {
            // 2. Add the current one unless the user has already added it by hand
            if (path_rewrite(work, dir, NULL) == 0)
            {
                if (work[0] != '\0')
                    strcat(work, ";");
                strcat(work, dir);
            }
        }
        if (strcmp(work, old) != 0)
        {
            reg_write(env, "Path", type, work);
            changed = 1;
        }
        // 3. Remember (or forget) the directory
        if (op == PATH_ADD)
        {
            if (RegCreateKeyExA(HKEY_CURRENT_USER, APP_KEY, 0, NULL, 0, KEY_SET_VALUE, NULL, &app, NULL) == ERROR_SUCCESS)
            {
                reg_write(app, "BinDir", REG_SZ, dir);
                RegCloseKey(app);
            }
            changed = 1;
        }
        else if (saved[0] != '\0')
        {
            RegDeleteKeyA(HKEY_CURRENT_USER, APP_KEY);
            changed = 1;
        }
        if (changed)
            broadcast_change();
    }
    RegCloseKey(env);
    free(saved);
    free(old);
    free(work);
    free(next);
    return result;
}

#else

#define MARK_BEGIN "# >>> refalab >>>"
#define MARK_END "# <<< refalab <<<"

// The names of the startup files where a block could have been written
static const char *const rc_files[] = {".profile", ".bashrc", ".bash_profile", ".zshrc"};

// Read a whole text file into a new string; NULL if there is no such file
static char *read_text(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    size_t cap = 4096, len = 0, n;
    char *text = malloc(cap);
    if (text == NULL)
        fail("out of memory", path);
    while ((n = fread(text + len, 1, cap - len - 1, f)) > 0)
    {
        len += n;
        if (len + 1 >= cap)
        {
            cap *= 2;
            text = realloc(text, cap);
            if (text == NULL)
                fail("out of memory", path);
        }
    }
    text[len] = '\0';
    fclose(f);
    return text;
}

// Quote dir for sh: 'dir', a single quote inside is written as '\''
static void sh_quote(const char *dir, char *quoted, size_t size)
{
    size_t n = 0;
    quoted[n++] = '\'';
    for (; *dir != '\0' && n + 5 < size; dir++)
    {
        if (*dir == '\'')
        {
            memcpy(quoted + n, "'\\''", 4);
            n += 4;
        }
        else
            quoted[n++] = *dir;
    }
    quoted[n++] = '\'';
    quoted[n] = '\0';
}

// Look through the RefalAB blocks of a startup file. If rewrite is not 0 the blocks are removed from it.
// Returns 1 if a block with the directory quoted was found.
static int rc_scan(const char *path, const char *quoted, int rewrite)
{
    char *text = read_text(path);
    if (text == NULL)
        return 0;
    char *out = malloc(strlen(text) + 1);
    size_t on = 0;
    int in_block = 0, found = 0, removed = 0;
    if (out == NULL)
        fail("out of memory", path);
    for (char *line = text; *line != '\0';)
    {
        char *eol = strchr(line, '\n');
        size_t ll = eol != NULL ? (size_t)(eol - line) + 1 : strlen(line);
        char saved = line[ll];
        line[ll] = '\0';
        if (!in_block && strncmp(line, MARK_BEGIN, strlen(MARK_BEGIN)) == 0)
        {
            in_block = 1;
            removed = 1;
        }
        if (in_block)
        {
            if (strstr(line, quoted) != NULL)
                found = 1;
            if (strncmp(line, MARK_END, strlen(MARK_END)) == 0)
                in_block = 0;
        }
        else
        {
            memcpy(out + on, line, ll);
            on += ll;
        }
        line[ll] = saved;
        line += ll;
    }
    if (rewrite && removed)
    {
        FILE *f = fopen(path, "wb");
        if (f == NULL)
            fprintf(stderr, "bootstrap: cannot write %s\n", path);
        else
        {
            fwrite(out, 1, on, f);
            fclose(f);
        }
    }
    free(text);
    free(out);
    return found;
}

// POSIX: a block in the startup file of the shell of the user.
// Returns 1 if dir was in PATH (in a block of RefalAB) before the operation.
static int user_path(const char *dir, int op)
{
    char quoted[1100], path[1100];
    int present = 0;
    const char *home = getenv("HOME");
    if (home == NULL)
    {
        fprintf(stderr, "bootstrap: HOME is not set, the startup files of the shell cannot be found\n");
        return 0;
    }
    sh_quote(dir, quoted, sizeof quoted);
    // PATH_ADD and PATH_REMOVE remove the old blocks from all the files, so there is never more than one
    for (size_t i = 0; i < sizeof rc_files / sizeof rc_files[0]; i++)
    {
        snprintf(path, sizeof path, "%s/%s", home, rc_files[i]);
        if (rc_scan(path, quoted, op != PATH_CHECK))
            present = 1;
    }
    if (op == PATH_ADD)
    {
        // zsh: ~/.zshrc; bash: ~/.bashrc (or ~/.bash_profile if only it exists); the others: ~/.profile
        const char *shell = getenv("SHELL");
        const char *rc = ".profile";
        if (shell != NULL && strstr(shell, "zsh") != NULL)
            rc = ".zshrc";
        else if (shell != NULL && strstr(shell, "bash") != NULL)
        {
            char other[1100];
            snprintf(other, sizeof other, "%s/.bashrc", home);
            rc = mtime(other) == 0 ? ".bash_profile" : ".bashrc";
        }
        snprintf(path, sizeof path, "%s/%s", home, rc);
        char *old = read_text(path);
        int need_newline = old != NULL && old[0] != '\0' && old[strlen(old) - 1] != '\n';
        free(old);
        FILE *f = fopen(path, "ab");
        if (f == NULL)
        {
            fprintf(stderr, "bootstrap: cannot write %s\n", path);
            return present;
        }
        fprintf(f, "%s%s\nexport PATH=%s:\"$PATH\"\n%s\n", need_newline ? "\n" : "", MARK_BEGIN, quoted, MARK_END);
        fclose(f);
        printf("Added to %s\n", path);
    }
    return present;
}

#endif

// After a successful build: offer to add the directory bin to PATH
static void offer_path(void)
{
    char dir[600], answer[16];
    bin_dir(dir, sizeof dir);
    if (user_path(dir, PATH_CHECK))
    {
        printf("%s is already in PATH\n", dir);
        return;
    }
    printf("Add %s to PATH? [y/N] ", dir);
    fflush(stdout);
    if (fgets(answer, sizeof answer, stdin) == NULL)
    {
        printf("\n"); // no answer (the input is closed): do not change PATH
        return;
    }
    if (answer[0] != 'y' && answer[0] != 'Y')
        return;
    user_path(dir, PATH_ADD);
    printf("Done. Open a new terminal for the change to take effect.\n");
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
            char dir[600];
            bin_dir(dir, sizeof dir);
            if (user_path(dir, PATH_REMOVE))
                printf("Removed %s from PATH. Open a new terminal for the change to take effect.\n", dir);
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
    offer_path();
    return 0;
}
