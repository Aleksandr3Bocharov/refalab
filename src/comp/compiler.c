// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-18
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compiler.c  ----------
//      The main file of RefalAB compiler
//---------------------------------------

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#if defined POSIX
#include <sys/wait.h>
#endif
#include "refalab.h"
#include "compiler.h"
#include "print_errors.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "identifiers.h"
#include "compile_sentence.h"

#ifndef CLANG_VERSION
#define CLANG_VERSION "unknown"
#endif

#ifndef CLANG_TARGET_TRIPLE
#define CLANG_TARGET_TRIPLE "unknown-target"
#endif

#ifndef CLANG_THREAD_MODEL
#define CLANG_THREAD_MODEL "unknown"
#endif

#define PRINT_ERROR_130 \
    print_error_string("130 invalid record format")

#define ns_b 0006
#define ns_cll 0000
#define ns_d 0013
#define ns_f 0007
#define ns_l 0014
#define ns_n 0010
#define ns_ng 0002
#define ns_ngw 0003
#define ns_o 0012
#define ns_r 0011
#define ns_s 0005
#define ns_sc 0004
#define ns_w 0001

#if defined POSIX
#define MAX_PATHFILENAME 4096
#else
#define MAX_PATHFILENAME 260
#endif

typedef enum module_states
{
    START_OF_MODULE,
    NEXT_STM,
    KEYS,
    END_IS_MISSING,
    END_STATEMENT,
    END_OF_SYSIN
} T_MODULE_STATES;

typedef enum scanner_states
{
    STATE0,
    SCNERR,
    SCNSC,
    EGO,
    SCNL,
    SCNR,
    SCNS,
    SCNW,
    SCNVV,
    SCNE,
    SCNV,
    SCNVD,
    SCNVI,
    SCNKK,
    SCNP,
    SCNEOL,
    SCNEOS,
    SCNA,
    STATE1,
    SCNCHR,
    STATE2,
    FSCN,
    NSCN,
    RSCN,
    OSCN,
    DSCN,
    LSCN,
    SABBR,
    OSH101,
    OSH102,
    OSH103,
    SOSH203,
    SOSH204,
    SCNGCR,
    SCNRET
} T_SCANNER_STATES;

typedef enum specifier_states
{
    SPCBLO,
    SPCPRC,
    SPCFF,
    SPCL,
    SPCR,
    SPCR1,
    SPCR2,
    SPCR3,
    SPCESC,
    SPCSP,
    SPCA,
    SPCA1,
    SPCES,
    SPCEB,
    SPCEW,
    SPCEF,
    SPCEN,
    SPCER,
    SPCEO,
    SPCEL,
    SPCED,
    SPCGC,
    OSH200,
    OSH202,
    OSH203,
    OSH204,
    OSH205,
    OSH206,
    OSH208
} T_SPECIFIER_STATES;

typedef struct timespec T_TIMESPEC;

T_OPTIONS options;

T_SCANNER scanner;

T_SENTENCE_ELEMENT current_sentence_element;

FILE *refalab_source_listing, *terminal;
FILE *llvm_source; // for llvm

static struct
{
    bool was_card_print_file_source_listing;
    bool was_error;
    bool was_card_print_terminal;
    bool now_compile_function; // now compile function flag
    bool compile_direction;    // L,R - flag
    bool scanner_station;      // scanner station - in(1),out(0) literal chain
    bool scanner_station_k;    // scanner station k - id(1),any(0) after k
    bool left_part_sentence;   // left part sentence flag
    bool end_refalab_source;   // "refalab_source" end flag
} flags;

static FILE *refalab_source;               // refalab source
static char *refalab_source_buffer = NULL; // refalab source buffer
static size_t refalab_source_size = 0;     // refalab source size
static size_t refalab_source_cursor = 0;   // refalab source cursor
static uint32_t errors_count;
static T_LABEL *specifier_abbreviated[7]; // abbreviated specifier table
static char statement_key[6];
static uint8_t statement_key_name_length;
static size_t module_length; // module length
static T_TIMESPEC time_begin;

static void load_refalab_source_to_memory(void);
static void get_statement_key(void);
static void blanks_out(void);
static void handle_identifiers_extern(void (*handler)(const char *, uint8_t, const char *, uint8_t));
static void handle_identifiers(void (*handler)(const char *, uint8_t));
static void equ(void);
static void print_conclusion(void);
static void print_card_refalab_source_listing(void);
static void generate_specifier(uint8_t n);
static bool compile_specifer(char tail);
static bool get_identifier(char *identifier, uint8_t *identifier_length);
static bool get_identifier_extern(char *identifier, uint8_t *identifier_length);
static bool get_multiple_symbol(T_LINKTI *code, char *identifier, uint8_t *identifier_length);

static inline char get_current_char(void)
{
    if (refalab_source_cursor == refalab_source_size)
    {
        flags.end_refalab_source = true;
        return '\0';
    }
    return *(refalab_source_buffer + refalab_source_cursor);
}

static inline void next_char(void)
{
    if (refalab_source_cursor < refalab_source_size)
    {
        if (*(refalab_source_buffer + refalab_source_cursor) == '\n')
        {
            scanner.carriage_number++;
            scanner.column_number = 0;
        }
        refalab_source_cursor++;
        scanner.column_number++;
    }
    else
        flags.end_refalab_source = true;
}

static inline void previous_char(void)
{
    if (scanner.column_number != 1)
    {
        refalab_source_cursor--;
        scanner.column_number--;
        flags.end_refalab_source = false;
    }
    return;
}

static inline void SET_time(void)
{
    clock_gettime(CLOCK_MONOTONIC, &time_begin);
    return;
}

static void GET_time(void)
{
    T_TIMESPEC time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    long int nanoseconds = time_end.tv_nsec - time_begin.tv_nsec;
    long long int seconds = (long long int)difftime(time_end.tv_sec, time_begin.tv_sec);
    if (nanoseconds < 0)
    {
        nanoseconds += 1000000000;
        seconds--;
    }
    long long int minutes = seconds / 60;
    seconds %= 60;
    char string_time[64];
    sprintf(string_time, "%02lld:%02lld.%09ld", minutes, seconds, nanoseconds);
    printf("                       elapsed time      = %s\n", string_time);
    return;
}

int main(int argc, char *argv[])
{
    terminal = NULL;
    scanner.module_number = 0;
    printf("\n");
    // Aleksandr Bocharov   // compiler version
    printf("RefalAB Version 2.0-dev 20260529 (c) Aleksandr Bocharov\n");
    printf("https://github.com/Aleksandr3Bocharov/refalab\n");
    printf("Compiled with clang version: %s\n", CLANG_VERSION);
    printf("Target: %s\n", CLANG_TARGET_TRIPLE);
    printf("Thread model: %s", CLANG_THREAD_MODEL);
    printf("\n");
    if (argc < 2)
    {
#if defined mdebug
        const char *compiler_name = "refalabc_dbg";
#else
        const char *compiler_name = "refalabc";
#endif
        printf("\nSyntax: %s source_file [option ... option]", compiler_name);
        printf("\nOptions:");
        printf("\n   -nn  No function names");
        printf("\n   -ns  No source listing");
        printf("\n   -fn  Full names");
        printf("\n   -ll  LLVM module");
        printf("\n   -l,[options] LLVM options");
        printf("\n\n");
        exit(1);
    };
    if (strlen(argv[1]) > MAX_PATHFILENAME - 4)
    {
        printf("\nSource file name too long\n");
        exit(1);
    }
    char filename[MAX_PATHFILENAME + 1];
    strcpy(filename, argv[1]);
    strcat(filename, ".ref");
    printf("\n%s:\n", filename);
    refalab_source = fopen(filename, Rbin);
    if (refalab_source == NULL)
    {
        printf("Can't open %s\n", filename);
        exit(1);
    };
    terminal = stdout;
    refalab_source_listing = NULL;
    SET_time();
    options.source_listing = true;
    options.full_name = false;
    options.names = true;
    options.llvm_source_only = false;
    options.llvm_options[0] = '\0';
    options.llvm_options[8191] = '\0';
    for (int j = 2; j < argc; ++j)
        if (strcmp(argv[j], "-nn") == 0)
            options.names = false;
        else if (strcmp(argv[j], "-ns") == 0)
            options.source_listing = false;
        else if (strcmp(argv[j], "-fn") == 0)
            options.full_name = true;
        else if (strcmp(argv[j], "-ll") == 0)
            options.llvm_source_only = true;
        else if (strncmp(argv[j], "-l,", 3) == 0)
            strncpy(options.llvm_options, &argv[j][3], 8191);
        else
        {
            printf("Unknown option: %s\n", argv[j]);
            printf("Options may be: -nn, -ns, -fn, -ll, -l,[options]\n");
            exit(1);
        }
    if (options.source_listing)
    {
        strcpy(filename, argv[1]);
        strcat(filename, ".lst");
        refalab_source_listing = fopen(filename, "w");
        if (refalab_source_listing == NULL)
        {
            printf("Can't open %s\n", filename);
            exit(8);
        }
    }
    strcpy(filename, argv[1]);
    strcat(filename, ".ll");
    llvm_source = fopen(filename, "w");
    if (llvm_source == NULL)
    {
        printf("Can't open %s\n", filename);
        exit(8);
    }
    flags.was_error = false;
    bool impl = false;
    T_MODULE_STATES module_state = START_OF_MODULE;
    while (true)
        switch (module_state)
        {
        case START_OF_MODULE:
            errors_count = 0;
            scanner.module_number++;
            flags.end_refalab_source = false;
            module_length = 0;
            for (uint8_t i = 0; i < 7; ++i)
                specifier_abbreviated[i] = NULL;
            // "start" - directive work
            load_refalab_source_to_memory();
            get_statement_key();
            if (flags.end_refalab_source)
            {
                module_state = END_OF_SYSIN;
                break;
            }
            module_init();
            if (strncmp(statement_key, "START", statement_key_name_length) != 0)
            {
                print_error_string("001 START-directive missing");
                scanner.module_name_length = 0;
                macrocode_start();
                module_state = KEYS;
                break;
            }
            memcpy(scanner.module_name, statement_label, statement_label_length);
            scanner.module_name_length = statement_label_length;
            macrocode_start();
            blanks_out();
            if (current_symbol_number != CUT - 1 || symbols[current_symbol_number] != ' ')
                PRINT_ERROR_130;
            module_state = NEXT_STM;
            break;
        case NEXT_STM:
            // read of next sentence
            label_key(false);
            module_state = KEYS;
            break;
        case KEYS:
            if (strncasecmp(statement_key, "impl", 4) == 0)
            {
                if (impl == true)
                    print_error_string("011 impl-directive in the impl-section");
                if (statement_label_length != 0)
                    PRINT_ERROR_130;
                else
                {
                    blanks_out();
                    if (current_symbol_number != CUT - 1 || symbols[current_symbol_number] != ' ')
                        PRINT_ERROR_130;
                };
                impl = true;
            }
            else if (strncasecmp(statement_key, "l ", 2) == 0)
            {
                if (impl == false)
                    print_error_string("021 l-directive not in the impl-section");
                flags.compile_direction = true;
                previous_label_to_statement_label();
                compile_sentence(flags.compile_direction, statement_label, statement_label_length);
            }
            else if (strncasecmp(statement_key, "r ", 2) == 0)
            {
                if (impl == false)
                    print_error_string("022 r-directive not in the impl-section");
                flags.compile_direction = false;
                previous_label_to_statement_label();
                compile_sentence(flags.compile_direction, statement_label, statement_label_length);
            }
            else if (strncasecmp(statement_key, "start", 5) == 0)
            {
                if (impl == true)
                    print_error_string("012 start-directive in the impl-section");
                else
                    print_error_string("002 too many start-directive");
                blanks_out();
                if (current_symbol_number != CUT - 1 || symbols[current_symbol_number] != ' ')
                    PRINT_ERROR_130;
            }
            else if (strncasecmp(statement_key, "end", 3) == 0)
            {
                if (statement_label_length != 0)
                    PRINT_ERROR_130;
                else
                {
                    blanks_out();
                    if (current_symbol_number != CUT - 1 || symbols[current_symbol_number] != ' ')
                        PRINT_ERROR_130;
                }
                module_state = END_STATEMENT;
                break;
            }
            else if (strncasecmp(statement_key, "entry", 5) == 0)
            {
                if (impl == true)
                    print_error_string("013 entry-directive in the impl-section");
                handle_identifiers_extern(set_entry);
            }
            else if (strncasecmp(statement_key, "extrn", 5) == 0)
            {
                if (impl == true)
                    print_error_string("014 extrn-directive in the impl-section");
                handle_identifiers_extern(set_extrn);
            }
            else if (strncasecmp(statement_key, "empty", 5) == 0)
            {
                if (impl == true)
                    print_error_string("015 empty-directive in the impl-section");
                handle_identifiers(set_empty);
            }
            else if (strncasecmp(statement_key, "swap", 4) == 0)
            {
                if (impl == true)
                    print_error_string("016 swap-directive in the impl-section");
                handle_identifiers(set_swap);
            }
            else if (strncasecmp(statement_key, "s ", 2) == 0)
            {
                if (impl == true)
                    print_error_string("017 s-directive in the impl-section");
                specifier_definition(statement_label, statement_label_length);
                compile_specifer(' ');
            }
            else if (strncasecmp(statement_key, "equ", 3) == 0)
            {
                if (impl == true)
                    print_error_string("018 equ-directive in the impl-section");
                equ();
            }
            else if (statement_key[0] == ' ')
            {
                previous_label_to_statement_label();
                if (statement_label_length != 0)
                {
                    if (impl == false)
                        print_error_string("023 function not in the impl-section");
                    strncpy(previous_label, statement_label, statement_label_length);
                    previous_label[statement_label_length] = '\0';
                }
            }
            else
            {
                if (impl == false)
                    print_error_string("021 l-directive not in the impl-section");
                flags.compile_direction = true;
                previous_label_to_statement_label();
                compile_sentence(flags.compile_direction, statement_label, statement_label_length);
            }
            if (!flags.end_refalab_source)
            {
                module_state = NEXT_STM;
                break;
            }
            module_state = END_IS_MISSING;
            break;
        case END_IS_MISSING:
            print_error_string("003 end-directive missing");
            errors_count++;
            module_state = END_STATEMENT;
            break;
        case END_STATEMENT:
            module_end();
            if (errors_count != 0)
            {
                flags.was_error = true;
                module_length = 0;
            }
            else
            {
                macrocode_end();
                module_length = macrocode_where();
            }
            module_terminate();
            print_conclusion();
            module_state = END_OF_SYSIN;
            break;
        case END_OF_SYSIN:
            free(refalab_source_buffer);
            refalab_source_buffer = NULL;
            fclose(refalab_source);
            if (options.source_listing)
                fclose(refalab_source_listing);
            module_length = macrocode_where();
            fclose(llvm_source);
            if (module_length == 0 || flags.was_error)
                unlink(filename);
            else if (!options.llvm_source_only)
            {
                char string_llvm[9 + MAX_PATHFILENAME + 1 + 8191 + 1];
                sprintf(string_llvm, "clang -c %s %s", filename, options.llvm_options);
                int result_as = system(string_llvm);
#if defined POSIX
                if (WIFEXITED(result_as) != 0)
                    result_as = WEXITSTATUS(result_as);
                else
                    result_as = -1;
#endif
                if (result_as != 0)
                {
                    printf("Error compiling to object file\n");
                    exit(1);
                }
            }
            if (flags.was_error)
                exit(1);
            else
                exit(0);
            return 0;
        }
} // main program  end

static void load_refalab_source_to_memory(void)
{
    if (refalab_source_buffer != NULL)
    {
        free(refalab_source_buffer);
        refalab_source_buffer = NULL;
    }
    scanner.carriage_number = 0;
    scanner.column_number = 0;
    fseek(refalab_source, 0, SEEK_END);
    long int file_size = ftell(refalab_source);
    fseek(refalab_source, 0, SEEK_SET);
    if (file_size <= 0)
    {
        flags.end_refalab_source = true;
        return;
    }
    refalab_source_buffer = (char *)malloc(file_size + 1);
    if (refalab_source_buffer == NULL)
    {
        printf("Out of memory\n");
        exit(1);
    }
    size_t read_bytes = fread(refalab_source_buffer, 1, file_size, refalab_source);
    *(refalab_source_buffer + read_bytes) = '\0';
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < read_bytes; read_index++)
    {
        if (refalab_source_buffer[read_index] == '\r' && read_index + 1 < read_bytes && refalab_source_buffer[read_index + 1] == '\n')
            continue;
        refalab_source_buffer[write_index++] = refalab_source_buffer[read_index];
    }
    refalab_source_buffer[write_index] = '\0';
    refalab_source_size = write_index;
    refalab_source_cursor = 0;
    scanner.carriage_number++;
    scanner.column_number++;
    flags.end_refalab_source = false;
}

static void get_statement_key(void)
{
    statement_key_name_length = 0;
    memset(statement_key, ' ', 6);
    blanks_out();
    if (get_current_char() != '$')
    {
        print_error_string("004 directive missing");
        return;
    }
    next_char();
    char current_char = get_current_char();
    if (isalpha((unsigned char)current_char) == 0)
    {
        print_error_string("005 directive empty");
        return;
    }
    statement_key[0] = (char)toupper((unsigned char)current_char);
    for (statement_key_name_length = 1; statement_key_name_length < 6; statement_key_name_length++)
    {
        next_char();
        current_char = get_current_char();
        if (isalpha((unsigned char)current_char) == 0)
            return;
        statement_key[statement_key_name_length] = (char)toupper((unsigned char)current_char);
    }
    size_t i = 0;
    while (isalpha((unsigned char)current_char) != 0)
    {
        next_char();
        current_char = get_current_char();
        i++;
    }
    if (i > 1)
    {
        statement_key_name_length = 0;
        print_error_string("007 Too long directive");
        return;
    }
    return;
}

void scan_sentence_element(void)
{
    char identifier[MAX_IDENTIFIER_LENGTH];
    uint8_t identifier_length;
    uint8_t specifier_code = 0;
    current_sentence_element.code.tag = TAGO;
    current_sentence_element.code.info.codef = NULL;
    current_sentence_element.v_variable = false;
    current_sentence_element.specifier.tag = TAGO;
    current_sentence_element.specifier.info.codef = NULL;
    T_SCANNER_STATES scanner_state = STATE0;
    if (flags.scanner_station)
        scanner_state = STATE1;
    if (flags.scanner_station_k)
        scanner_state = STATE2;
    char current_char = get_current_char();
    while (true)
        switch (scanner_state)
        {
        case STATE0:
            // among elements
            blanks_out();
            current_char = get_current_char();
            switch (current_char)
            {
            case '&':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                scanner_state = SCNSC;
                break;
            case '(':
                scanner_state = SCNL;
                break;
            case ')':
                scanner_state = SCNR;
                break;
            case 's':
            case 'S':
                scanner_state = SCNS;
                break;
            case 'w':
            case 'W':
                scanner_state = SCNW;
                break;
            case 'v':
            case 'V':
                scanner_state = SCNVV;
                break;
            case 'e':
            case 'E':
                scanner_state = SCNE;
                break;
            case '<':
                scanner_state = SCNKK;
                break;
            case '>':
                scanner_state = SCNP;
                break;
            case '=':
                scanner_state = SCNEOL;
                break;
            case ';':
                scanner_state = SCNEOS;
                break;
            case '\'':
                scanner_state = SCNA;
                break;
            case 'f':
            case 'F':
                scanner_state = FSCN;
                break;
            case 'n':
            case 'N':
                scanner_state = NSCN;
                break;
            case 'r':
            case 'R':
                scanner_state = RSCN;
                break;
            case 'o':
            case 'O':
                scanner_state = OSCN;
                break;
            case 'd':
            case 'D':
                scanner_state = DSCN;
                break;
            case 'l':
            case 'L':
                scanner_state = LSCN;
                break;
            default:
                print_error_string_symbol("100 illegal symbol", current_char);
                scanner_state = SCNERR;
            }
            break;
        case SCNERR:
            current_sentence_element.type = NONE;
            scanner_state = SCNRET;
            break;
        case SCNSC:
            if (get_multiple_symbol(&current_sentence_element.code, identifier, &identifier_length))
            {
                scanner_state = EGO;
                break;
            }
            scanner_state = SCNERR;
            break;
        case EGO:
            current_sentence_element.type = SC;
            scanner_state = SCNRET;
            break;
        case SCNL:
            current_sentence_element.type = LB;
            scanner_state = SCNGCR;
            break;
        case SCNR:
            current_sentence_element.type = RB;
            scanner_state = SCNGCR;
            break;
        case SCNS:
            current_sentence_element.type = S_V;
            scanner_state = SCNV;
            break;
        case SCNW:
            current_sentence_element.type = W_V;
            scanner_state = SCNV;
            break;
        case SCNVV:
            current_sentence_element.v_variable = true;
            scanner_state = SCNE;
            break;
        case SCNE:
            current_sentence_element.type = E_V;
            scanner_state = SCNV;
            break;
        case SCNV:
            next_char();
            current_char = get_current_char();
            if (current_char == '(')
            {
                next_char();
                if (flags.left_part_sentence)
                {
                    current_sentence_element.specifier.info.codef = (T_LABEL *)generate_info_label();
                    macrocode_label(current_sentence_element.specifier.info.codef);
                }
                if (compile_specifer(')'))
                    next_char();
            }
            else if (current_char == ':')
            {
                next_char();
                if (!get_identifier(identifier, &identifier_length))
                {
                    scanner_state = SOSH203;
                    break;
                }
                if (flags.left_part_sentence)
                    current_sentence_element.specifier.info.codef = specifier_reference(identifier, identifier_length, ')');
                if (get_current_char() == ':')
                    next_char();
            }
            scanner_state = SCNVD;
            break;
        case SCNVD:
            if (get_current_char() != '.')
            {
                scanner_state = OSH103;
                break;
            }
            next_char();
            scanner_state = SCNVI;
            break;
        case SCNVI:
            if (!get_identifier(identifier, &identifier_length))
            {
                scanner_state = OSH102;
                break;
            }
            strncpy(current_sentence_element.identifier, identifier, identifier_length);
            current_sentence_element.identifier_length = identifier_length;
            scanner_state = SCNRET;
            break;
        case SCNKK:
            current_sentence_element.type = K;
            next_char();
            if (isspace((unsigned char)get_current_char()) == 0)
                flags.scanner_station_k = true;
            scanner_state = SCNRET;
            break;
        case SCNP:
            current_sentence_element.type = DOT;
            scanner_state = SCNGCR;
            break;
        case SCNEOL:
            current_sentence_element.type = EQ;
            flags.left_part_sentence = false;
            scanner_state = SCNGCR;
            break;
        case SCNEOS:
            current_sentence_element.type = END;
            scanner_state = SCNRET;
            break;
        case SCNA:
            next_char();
            current_char = get_current_char();
            if (current_char == '\n' || current_char == '\0')
            {
                scanner_state = OSH101;
                break;
            }
            if (current_char == '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            flags.scanner_station = true;
            scanner_state = SCNCHR;
            break;
        case STATE1: // within letter chain
            if (current_char == '\n' || current_char == '\0')
            {
                scanner_state = OSH101;
                break;
            }
            if (current_char != '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            next_char();
            if (get_current_char() == '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            flags.scanner_station = false;
            scanner_state = STATE0;
            break;
        case SCNCHR:
            current_sentence_element.code.tag = TAGO;
            current_sentence_element.code.info.codef = NULL;
            char symbol = get_current_char();
            if (symbol == '\\')
            // control symbols
            {
                next_char();
                char temp_symbol = get_current_char();
                switch (temp_symbol)
                {
                case '\\':
                    break;
                case 'n':
                    symbol = '\012';
                    break;
                case 't':
                    symbol = '\011';
                    break;
                case 'v':
                    symbol = '\013';
                    break;
                case 'r':
                    symbol = '\015';
                    break;
                case 'f':
                    symbol = '\014';
                    break;
                case '0':
                    next_char();
                    temp_symbol = get_current_char();
                    if (temp_symbol >= '0' && temp_symbol <= '7')
                    {
                        uint32_t octal = (uint32_t)temp_symbol - '0';
                        next_char();
                        temp_symbol = get_current_char();
                        if (temp_symbol >= '0' && temp_symbol <= '7')
                            octal = octal * 8 + (uint32_t)temp_symbol - '0';
                        else
                            previous_char();
                        symbol = (char)(octal & 255);
                    }
                    else
                    {
                        previous_char();
                        symbol = '\0';
                    }
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    uint32_t octal = (uint32_t)temp_symbol - '0';
                    for (uint8_t i = 1; i < 3; i++)
                    {
                        next_char();
                        temp_symbol = get_current_char();
                        if (temp_symbol >= '0' && temp_symbol <= '7')
                            octal = octal * 8 + (uint32_t)temp_symbol - '0';
                        else
                            previous_char();
                    }
                    symbol = (char)(octal & 255);
                    break;
                case '\n':
                case '\0':
                    previous_char();
                    break;
                default:
                    symbol = temp_symbol;
                }
            }
            current_sentence_element.code.info.infoc = symbol;
            current_sentence_element.type = SC;
            scanner_state = SCNGCR;
            break;
        case STATE2:
            flags.scanner_station_k = false;
            current_sentence_element.code.tag = TAGO;
            current_sentence_element.code.info.codef = NULL;
            if (get_identifier(identifier, &identifier_length))
            {
                current_sentence_element.code.info.codef = function_reference(identifier, identifier_length);
                current_sentence_element.code.tag = TAGF;
                scanner_state = EGO;
                break;
            }
            print_error_string_symbol("100 illegal symbol", get_current_char());
            scanner_state = SCNERR;
            break;
        case FSCN:
            specifier_code = 0;
            scanner_state = SABBR;
            break;
        case NSCN:
            specifier_code = 1;
            scanner_state = SABBR;
            break;
        case RSCN:
            specifier_code = 2;
            scanner_state = SABBR;
            break;
        case OSCN:
            specifier_code = 3;
            scanner_state = SABBR;
            break;
        case DSCN:
            specifier_code = 4;
            scanner_state = SABBR;
            break;
        case LSCN:
            specifier_code = 5;
            scanner_state = SABBR;
            break;
        case SABBR:
            current_sentence_element.type = S_V;
            if (flags.left_part_sentence)
            {
                if (*(specifier_abbreviated + specifier_code) == NULL)
                {
                    *(specifier_abbreviated + specifier_code) = (T_LABEL *)generate_info_label();
                    macrocode_label(*(specifier_abbreviated + specifier_code));
                    generate_specifier(specifier_code + 7);
                    generate_specifier(ns_ngw);
                };
                current_sentence_element.specifier.info.codef = *(specifier_abbreviated + specifier_code);
            };
            next_char();
            scanner_state = SCNVD;
            break;
        case OSH101:
            print_error_string("101 default of left apostroph");
            scanner_state = SCNERR;
            break;
        case OSH102:
            print_error_string("102 sign '.' followed by no letter or underscore");
            scanner_state = SCNERR;
            break;
        case OSH103:
            print_error_string("103 sign '.' expected");
            scanner_state = SCNERR;
            break;
        case SOSH203:
            print_error_string("203 sign ':' followed by no letter or underscore");
            scanner_state = SCNERR;
            break;
        case SOSH204:
            print_error_string("204 default last ':' within specifier");
            scanner_state = SCNERR;
            break;
        case SCNGCR:
            next_char();
            scanner_state = SCNRET;
            break;
        case SCNRET:
            return;
        }
}

static void generate_specifier(uint8_t n)
{
    if (flags.left_part_sentence)
        macrocode_byte(n);
    return;
}

static bool compile_specifer(char tail)
{ // specifier compiler
    bool negative = false;
    char identifier[MAX_IDENTIFIER_LENGTH];
    uint8_t identifier_length;
    T_LINKTI code;
    T_SPECIFIER_STATES specifier_state = SPCBLO;
    char current_char = get_current_char();
    while (true)
        switch (specifier_state)
        {
        case SPCBLO:
            blanks_out();
            specifier_state = SPCPRC;
            break;
        case SPCPRC:
            current_char = get_current_char();
            switch (current_char)
            {
            case ';':
                specifier_state = SPCFF;
                break;
            case '(':
                specifier_state = SPCL;
                break;
            case ')':
                specifier_state = SPCR;
                break;
            case '&':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                specifier_state = SPCESC;
                break;
            case ':':
                specifier_state = SPCSP;
                break;
            case '\'':
                specifier_state = SPCA;
                break;
            case 's':
            case 'S':
                specifier_state = SPCES;
                break;
            case 'b':
            case 'B':
                specifier_state = SPCEB;
                break;
            case 'w':
            case 'W':
                specifier_state = SPCEW;
                break;
            case 'f':
            case 'F':
                specifier_state = SPCEF;
                break;
            case 'n':
            case 'N':
                specifier_state = SPCEN;
                break;
            case 'r':
            case 'R':
                specifier_state = SPCER;
                break;
            case 'o':
            case 'O':
                specifier_state = SPCEO;
                break;
            case 'l':
            case 'L':
                specifier_state = SPCEL;
                break;
            case 'd':
            case 'D':
                specifier_state = SPCED;
                break;
            default:
                print_error_string_symbol("201 within specifier invalid symbol ", current_char);
                specifier_state = OSH200;
            }
            break;
        case SPCFF:
            generate_specifier(ns_ngw);
            if (negative)
                print_error_string("207 within specifier default ')' ");
            if (tail == ')')
            {
                specifier_state = OSH206;
                break;
            }
            return true;
        case SPCL:
            if (negative)
            {
                specifier_state = OSH202;
                break;
            }
            negative = true;
            generate_specifier(ns_ng);
            specifier_state = SPCGC;
            break;
        case SPCR:
            if (!negative)
            {
                specifier_state = SPCR1;
                break;
            }
            next_char();
            blanks_out();
            current_char = get_current_char();
            if (current_char == '(')
            {
                specifier_state = SPCGC;
                break;
            }
            if (current_char == ')')
            {
                specifier_state = SPCR1;
                break;
            }
            if (current_char == ';')
            {
                specifier_state = SPCR2;
                break;
            }
            negative = false;
            generate_specifier(ns_ng);
            specifier_state = SPCPRC;
            break;
        case SPCR1:
            if (tail == ')')
            {
                specifier_state = SPCR3;
                break;
            }
            specifier_state = OSH208;
            break;
        case SPCR2:
            if (tail != ')')
            {
                specifier_state = SPCR3;
                break;
            }
            specifier_state = OSH206;
            break;
        case SPCR3:
            generate_specifier(ns_ngw);
            return true;
        case SPCESC:
            if (!get_multiple_symbol(&code, identifier, &identifier_length))
            {
                specifier_state = OSH200;
                break;
            }
            generate_specifier(ns_sc);
            if (flags.left_part_sentence)
                generate_symbol(&code);
            specifier_state = SPCBLO;
            break;
        case SPCSP:
            next_char();
            if (!get_identifier(identifier, &identifier_length))
            {
                specifier_state = OSH203;
                break;
            }
            if (strncmp(statement_label, identifier, identifier_length) == 0 && (identifier_length == MAX_IDENTIFIER_LENGTH || statement_label[identifier_length] == ' '))
                print_error_string("209 specifier is defined through itself");
            T_LABEL *identifier_specifier = specifier_reference(identifier, identifier_length, tail);
            generate_specifier(ns_cll);
            if (flags.left_part_sentence)
                macrocode_address(identifier_specifier);
            if (get_current_char() == ':')
            {
                specifier_state = SPCGC;
                break;
            }
            specifier_state = OSH204;
            break;
        case SPCA:
            next_char();
            current_char = get_current_char();
            if (current_char == '\n' || current_char == '\0')
            {
                specifier_state = OSH205;
                break;
            }
            if (current_char != '\'')
            {
                specifier_state = SPCA1;
                break;
            }
            generate_specifier(ns_sc);
            if (flags.left_part_sentence)
            {
                code.tag = TAGO;
                code.info.codef = NULL;
                code.info.infoc = '\'';
                generate_symbol(&code);
            }
            specifier_state = SPCGC;
            break;
        case SPCA1:
            generate_specifier(ns_sc);
            if (flags.left_part_sentence)
            {
                char symbol = get_current_char();
                if (symbol == '\\')
                {
                    next_char();
                    // control symbols ---------------
                    char temp_symbol = get_current_char();
                    switch (temp_symbol)
                    {
                    case '\\':
                        break;
                    case 'n':
                        symbol = '\012';
                        break;
                    case 't':
                        symbol = '\011';
                        break;
                    case 'v':
                        symbol = '\013';
                        break;
                    case 'r':
                        symbol = '\015';
                        break;
                    case 'f':
                        symbol = '\014';
                        break;
                    case '0':
                        next_char();
                        temp_symbol = get_current_char();
                        if (temp_symbol >= '0' && temp_symbol <= '7')
                        {
                            uint32_t octal = (uint32_t)temp_symbol - '0';
                            next_char();
                            temp_symbol = get_current_char();
                            if (temp_symbol >= '0' && temp_symbol <= '7')
                                octal = octal * 8 + (uint32_t)temp_symbol - '0';
                            else
                                previous_char();
                            symbol = (char)(octal & 255);
                        }
                        else
                        {
                            previous_char();
                            symbol = '\0';
                        }
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        uint32_t octal = (uint32_t)temp_symbol - '0';
                        for (uint8_t i = 1; i < 3; i++)
                        {
                            next_char();
                            temp_symbol = get_current_char();
                            if (temp_symbol >= '0' && temp_symbol <= '7')
                                octal = octal * 8 + (uint32_t)temp_symbol - '0';
                            else
                                previous_char();
                        }
                        symbol = (char)(octal & 255);
                        break;
                    case '\n':
                    case '\0':
                        previous_char();
                        break;
                    default:
                        symbol = temp_symbol;
                    }
                }
                code.tag = TAGO;
                code.info.codef = NULL;
                code.info.infoc = symbol;
                generate_symbol(&code);
            }
            next_char();
            current_char = get_current_char();
            if (current_char == '\n' || current_char == '\0')
            {
                specifier_state = OSH205;
                break;
            }
            if (current_char != '\'')
                break;
            next_char();
            if (get_current_char() == '\'')
                break;
            specifier_state = SPCBLO;
            break;
        case SPCES:
            generate_specifier(ns_s);
            specifier_state = SPCGC;
            break;
        case SPCEB:
            generate_specifier(ns_b);
            specifier_state = SPCGC;
            break;
        case SPCEW:
            generate_specifier(ns_w);
            specifier_state = SPCGC;
            break;
        case SPCEF:
            generate_specifier(ns_f);
            specifier_state = SPCGC;
            break;
        case SPCEN:
            generate_specifier(ns_n);
            specifier_state = SPCGC;
            break;
        case SPCER:
            generate_specifier(ns_r);
            specifier_state = SPCGC;
            break;
        case SPCEO:
            generate_specifier(ns_o);
            specifier_state = SPCGC;
            break;
        case SPCEL:
            generate_specifier(ns_l);
            specifier_state = SPCGC;
            break;
        case SPCED:
            generate_specifier(ns_d);
            specifier_state = SPCGC;
            break;
        case SPCGC:
            next_char();
            specifier_state = SPCBLO;
            break;
        case OSH200:
            print_error_string("200 specifier is't scaned");
            return false;
        case OSH202:
            print_error_string("202 specifier has too many '(' ");
            specifier_state = OSH200;
            break;
        case OSH203:
            print_error_string("203 sign ':' followed by no letter or underscore within specifier ");
            specifier_state = OSH200;
            break;
        case OSH204:
            print_error_string("204 within specifier default last :");
            specifier_state = OSH200;
            break;
        case OSH205:
            print_error_string("205 within specifier default last apostroph");
            specifier_state = OSH200;
            break;
        case OSH206:
            print_error_string("206 default ')'in the specifier end ");
            specifier_state = OSH200;
            break;
        case OSH208:
            print_error_string("208 within specifier too many )");
            specifier_state = OSH200;
        }
}

static void print_card_refalab_source_listing(void)
{ // writing of card into refalab source listing
    if (!flags.was_card_print_file_source_listing && refalab_source_listing != NULL)
    {
        flags.was_card_print_file_source_listing = true;
        if (!flags.end_refalab_source)
        {
            char temp_string[CUT + 28];
            sprintf(temp_string, "%4zu %s", scanner.carriage_number, card);
            uint8_t i;
            for (i = CUT + 4; i > 4; i--)
                if (temp_string[i] != ' ')
                    break;
            i++;
            temp_string[i] = '\n';
            i++;
            temp_string[i] = '\0';
            fputs(temp_string, refalab_source_listing);
        }
    }
    return;
}

static void print_card_terminal(void)
{ // card writing into terminal
    if (!flags.was_card_print_terminal)
    {
        flags.was_card_print_terminal = true;
        card[CUT] = '\0';
        if (!flags.end_refalab_source)
        {
            char temp_string[CUT + 28];
            sprintf(temp_string, "%4zu %s\n", scanner.carriage_number, card);
            fputs(temp_string, terminal);
        }
    }
    return;
}

static void handle_identifiers(void (*handler)(const char *, uint8_t)) // treatment of directives having 'EMPTY' and 'SWAP' type
{
    if (statement_label_length != 0)
    {
        PRINT_ERROR_130;
        return;
    }
    blanks_out();
    while (true)
    {
        char identifier[MAX_IDENTIFIER_LENGTH];
        uint8_t identifier_length;
        if (!get_identifier(identifier, &identifier_length))
            break;
        (*handler)(identifier, identifier_length);
        blanks_out();
        const char current_char = get_current_char();
        if (current_char == ';')
            return;
        if (current_char == ',')
        {
            next_char();
            if (isspace((unsigned char)get_current_char()) != 0)
                blanks_out();
            continue;
        }
        break;
    }
    PRINT_ERROR_130;
    return;
}

static void handle_identifiers_extern(void (*handler)(const char *, uint8_t, const char *, uint8_t)) // treatment of directives having 'ENTRY' and 'EXTRN' type
{
    if (statement_label_length != 0)
    {
        PRINT_ERROR_130;
        return;
    }
    blanks_out();
    while (true)
    {
        char identifier[MAX_IDENTIFIER_LENGTH];
        uint8_t identifier_length;
        if (!get_identifier(identifier, &identifier_length))
            break;
        char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
        uint8_t identifier_extern_length;
        if (get_current_char() == '(')
        {
            next_char();
            if (!get_identifier_extern(identifier_extern, &identifier_extern_length))
                break;
            (*handler)(identifier, identifier_length, identifier_extern, identifier_extern_length);
            if (get_current_char() != ')')
                break;
            next_char();
        }
        else
        {
            identifier_extern_length = identifier_length > MAX_IDENTIFIER_EXTERN_LENGTH ? MAX_IDENTIFIER_EXTERN_LENGTH : identifier_length;
            memcpy(identifier_extern, identifier, identifier_extern_length);
            (*handler)(identifier, identifier_length, identifier_extern, identifier_extern_length);
        }
        blanks_out();
        const char current_char = get_current_char();
        if (current_char == ';')
            return;
        if (current_char == ',')
        {
            next_char();
            if (isspace((unsigned char)get_current_char()) != 0)
                blanks_out();
            continue;
        }
        break;
    }
    PRINT_ERROR_130;
    return;
}

static void equ(void)
{ // treatement of directives having 'EQU' type
    blanks_out();
    char identifier[MAX_IDENTIFIER_LENGTH];
    uint8_t identifier_length;
    do
    {
        if (!get_identifier(identifier, &identifier_length))
            break;
        set_equ(statement_label, statement_label_length, identifier, identifier_length);
        blanks_out();
        if (get_current_char() == ';')
            return;
    } while (false);
    PRINT_ERROR_130;
    return;
}

static bool get_multiple_symbol(T_LINKTI *code, char *identifier, uint8_t *identifier_length) // procedure read multiple symbol
{
    code->tag = TAGO;
    code->info.codef = NULL;
    do
    {
        char current_char = get_current_char();
        if (isdigit((unsigned char)current_char) != 0)
        {
            code->tag = TAGN;
            code->info.codef = NULL;
            code->info.coden = 0;
            uint64_t number = current_char - '0';
            bool multiple_symbol_end = false;
            while (true)
            {
                next_char();
                current_char = get_current_char();
                if (isdigit((unsigned char)current_char) == 0)
                {
                    code->tag = TAGN;
                    code->info.codef = NULL;
                    code->info.coden = (uint32_t)number;
                    multiple_symbol_end = true;
                    break;
                }
                const uint64_t remainder = current_char - '0';
                number = number * 10 + remainder;
                if (number <= MAX_NUMBER)
                    continue;
                break;
            }
            if (multiple_symbol_end)
                break;
            while (true)
            {
                next_char();
                if (isdigit((unsigned char)get_current_char()) != 0)
                    continue;
                break;
            }
            char error_111[64];
            sprintf(error_111, "111 symbol-number > %" PRIu32, MAX_NUMBER);
            print_error_string(error_111);
            break;
        }
        next_char();
        if (!get_identifier(identifier, identifier_length))
        {
            print_error_string("112 unknown type of the multiple symbol");
            return false;
        }
        code->info.codef = function_reference(identifier, *identifier_length);
        code->tag = TAGF;
    } while (false);
    return true;
}

static bool get_identifier(char *identifier, uint8_t *identifier_length)
{ // read identifier
    memset(identifier, ' ', MAX_IDENTIFIER_LENGTH);
    char current_char = get_current_char();
    if (isalpha((unsigned char)current_char) == 0 && current_char != '_')
        return false;
    identifier[0] = (char)toupper((unsigned char)current_char);
    for (*identifier_length = 1; *identifier_length < MAX_IDENTIFIER_LENGTH; (*identifier_length)++)
    {
        next_char();
        current_char = get_current_char();
        if (isalpha((unsigned char)current_char) == 0 && current_char != '_' && isdigit((unsigned char)current_char) == 0)
            return true;
        identifier[*identifier_length] = (char)toupper((unsigned char)current_char);
    }
    size_t i = 0;
    while (isalpha((unsigned char)current_char) != 0 || isdigit((unsigned char)current_char) != 0 || current_char == '_')
    {
        next_char();
        current_char = get_current_char();
        i++;
    }
    if (i > 1)
    {
        char errror_113[64];
        sprintf(errror_113, "113 identifier length > %d", MAX_IDENTIFIER_LENGTH);
        print_error_string(errror_113);
    }
    return true;
}

// read external identifier
static bool get_identifier_extern(char *identifier, uint8_t *identifier_length)
{
    char current_char = get_current_char();
    if (isalpha((unsigned char)current_char) == 0 && current_char != '_')
        return false;
    identifier[0] = (char)toupper((unsigned char)current_char);
    for (*identifier_length = 1; *identifier_length < MAX_IDENTIFIER_EXTERN_LENGTH; (*identifier_length)++)
    {
        next_char();
        current_char = get_current_char();
        if (isalpha((unsigned char)current_char) == 0 && current_char != '_' && isdigit((unsigned char)current_char) == 0)
            return true;
        identifier[*identifier_length] = (char)toupper((unsigned char)get_current_char());
    }
    size_t i = 0;
    while (isalpha((unsigned char)current_char) != 0 || isdigit((unsigned char)current_char) != 0 || current_char == '_')
    {
        next_char();
        current_char = get_current_char();
        i++;
    }
    if (i > 1)
    {
        char error_114[64];
        sprintf(error_114, "114 external identifier length > %d", MAX_IDENTIFIER_EXTERN_LENGTH);
        print_error_string(error_114);
    }
    return true;
}

static void blanks_out(void)
{
    while (flags.end_refalab_source == false)
    {
        char symbol = get_current_char();
        if (isspace((unsigned char)symbol) != 0)
            next_char();
        else if (symbol == '*' && scanner.column_number == 1)
            while (flags.end_refalab_source == false && get_current_char() != '\n')
                next_char();
        else
            break;
    }
}

static void print_conclusion(void)
{ // print conclusion
    char print_line[180];
    sprintf(print_line,
            "module_name = %-40s    module_length(lines) = %zu\n", scanner.module_name, scanner.carriage_number);
    if (options.source_listing)
        fputs(print_line, refalab_source_listing);
    fputs(print_line, terminal);
    if (errors_count != 0)
        sprintf(print_line,
                "errors   = %-3d         module_obj_length(bytes) = %zu\n", errors_count, module_length);
    else
        sprintf(print_line,
                "                       module_obj_length(bytes) = %zu\n", module_length);
    if (options.source_listing)
        fputs(print_line, refalab_source_listing);
    fputs(print_line, terminal);
    GET_time();
    return;
}

void processing_error(void)
{
    print_card_refalab_source_listing();
    print_card_terminal();
    errors_count++;
    return;
}

//----------  end of file compiler.c  ----------
