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
//     Windows: the user variable Path in the registry (HKCU\\Environment);
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
    char objdir[128], cmd[CMD_SIZE], obj[512], src[512], ar[CMD_SIZE];
    time_t hdr = newest("src/inter", ".h");
    int nc = list_files("src/inter", ".c", cnames);
    int nr = list_files("src/inter", ".ref", rnames);

    printf("=== Building interpreter (%s) ===\n", v->name);
    snprintf(objdir, sizeof objdir, "build/%s/inter", v->name);
    make_dir(objdir);
    snprintf(src, sizeof src, "build/%s/tmp", v->name); // the temporary copies of the .ref modules
    make_dir(src);
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
            compile_ref(v, rnames[i], obj);
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

// Windows: the user variable Path in the registry (the type REG_EXPAND_SZ is kept).
// Returns 1 if dir was in Path before the operation.
static int user_path(const char *dir, int op)
{
    HKEY key;
    DWORD type = REG_EXPAND_SZ, size = 0;
    size_t dl = strlen(dir);
    int present = 0;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &key) != ERROR_SUCCESS)
    {
        fprintf(stderr, "bootstrap: cannot open the user environment in the registry\n");
        return 0;
    }
    if (RegQueryValueExA(key, "Path", NULL, &type, NULL, &size) != ERROR_SUCCESS)
    {
        size = 0;
        type = REG_EXPAND_SZ;
    }
    else if (type != REG_SZ && type != REG_EXPAND_SZ)
        type = REG_EXPAND_SZ;
    char *old = calloc((size_t)size + 2, 1);
    char *updated = calloc((size_t)size + dl + 4, 1);
    if (old == NULL || updated == NULL)
        fail("out of memory", "PATH");
    if (size > 0)
    {
        DWORD got = size;
        RegQueryValueExA(key, "Path", NULL, NULL, (LPBYTE)old, &got);
    }
    // Copy the entries one by one, dropping the empty ones and (for PATH_REMOVE) the entry dir
    for (const char *p = old; *p != '\0';)
    {
        const char *end = strchr(p, ';');
        size_t l = end != NULL ? (size_t)(end - p) : strlen(p);
        size_t k = l;
        while (k > 0 && (p[k - 1] == '\\' || p[k - 1] == '/'))
            k--;
        int match = k == dl && _strnicmp(p, dir, k) == 0;
        if (match)
            present = 1;
        if (l > 0 && !(match && op == PATH_REMOVE))
        {
            if (updated[0] != '\0')
                strcat(updated, ";");
            strncat(updated, p, l);
        }
        p += l;
        if (*p == ';')
            p++;
    }
    if (op == PATH_ADD && !present)
    {
        if (updated[0] != '\0')
            strcat(updated, ";");
        strcat(updated, dir);
    }
    if ((op == PATH_ADD && !present) || (op == PATH_REMOVE && present))
    {
        if (RegSetValueExA(key, "Path", 0, type, (const BYTE *)updated, (DWORD)strlen(updated) + 1) != ERROR_SUCCESS)
            fprintf(stderr, "bootstrap: cannot write the user Path to the registry\n");
        // Let the running programs (Explorer, new terminals) see the change
        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);
    }
    RegCloseKey(key);
    free(old);
    free(updated);
    return present;
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
