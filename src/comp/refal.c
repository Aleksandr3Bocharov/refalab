// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  REFAL.C -------------
//      The main file of RefalAB compiler
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "refalab.h"
#include "refal.h"
#include "cerr.h"
#include "cj.h"
#include "cgop.h"
#include "cs.h"
#include "ccst.h"

#define CUT 88

#define EH_ROMA                                \
    if (m != CUT - 1)                          \
    {                                          \
        m++;                                   \
        if (m == CUT - 1 && c[CUT - 1] != ' ') \
        {                                      \
            rdcard();                          \
            if (_eoj)                          \
                return;                        \
        }                                      \
    }

#define EH_ROMA0                               \
    if (m != CUT - 1)                          \
    {                                          \
        m++;                                   \
        if (m == CUT - 1 && c[CUT - 1] != ' ') \
        {                                      \
            rdcard();                          \
            if (_eoj)                          \
                return false;                  \
        }                                      \
    }

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
    PROD,
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

T_OPTIONS options;

T_SCANNER scanner;

T_SENTENCE_ELEMENT current_sentence_element;

FILE *refalab_source_listing, *terminal;
FILE *assembler_source; // for assem

static struct
{
    bool was_cut;
    bool was_card_print_file_source_listing;
    bool was_error;
    bool was_card_print_terminal;
} flags;

// Aleksandr Bocharov   // compiler version
static const char *version = "RefalAB Version 1.4-dev 20260309 (c) Aleksandr Bocharov";

static FILE *refalab_source;
static size_t m;           // current symbol number
static bool empcard;       // flags for empty card
static char card[CUT + 9]; // card buffer (input)
static const char *card_cut = card;
static uint32_t cdnumb; // card number
static bool dir;        // L,R - flag
static uint32_t kolosh;
static char strg_c[CUT + 6];
static char *c = strg_c + 6;
static char class_cut[CUT + 6];
static char *class = class_cut + 6;
static bool scn_station; // scanner station - in(1),out(0) literal chain
static bool left_part;
static char *sarr[7]; // abbreviated specifier table
static char stmlbl[MAX_IDENTIFIER_LENGTH];
static size_t lbl_leng;
static char prevlb[MAX_IDENTIFIER_LENGTH + 1];
static char stmkey[6];
static size_t fixm;                   // start sentence position
static char mod_name[MAX_IDENTIFIER_LENGTH + 1]; // module name
static size_t mod_length;             // module length
static bool _eoj;                     // "sysin" end flag

static void lblkey(bool pr);
static void pch130(void);
static void blout(void);
static void trprev(void);
static void ilm(void (*prog)(const char *, size_t, const char *, size_t));
static void il(void (*prog)(const char *, size_t));
static void equ(void);
static void pchzkl(void);
static void pchk(void);
static void gsp(uint8_t n);
static bool specif(char tail);
static bool get_id(char id[MAX_IDENTIFIER_LENGTH], size_t *lid);
static bool get_idm(char id[MAX_EXTERN_IDENTIFIER_LENGTH], size_t *lid);
static bool get_csmb(T_LINKTI *code, char id[MAX_IDENTIFIER_LENGTH], size_t *lid);

typedef struct timespec T_TIMESPEC;
static T_TIMESPEC t0;

static void SET_time(void)
{
    timespec_get(&t0, TIME_UTC);
    return;
}

static void GET_time(void)
{
    T_TIMESPEC t1;
    timespec_get(&t1, TIME_UTC);
    long int in = t1.tv_nsec - t0.tv_nsec;
    long long int is = (long long int)difftime(t1.tv_sec, t0.tv_sec);
    if (in < 0)
    {
        in += 1000000000;
        is--;
    }
    long long int im = is / 60;
    is %= 60;
    char s[64];
    sprintf(s, "%02lld:%02lld.%09ld", im, is, in);
    printf("                       elapsed time      = %s\n", s);
    return;
}

int main(int argc, char *argv[])
{
    terminal = NULL;
    scanner.module_number = 0;
    printf("\n");
    printf("%s", version);
    if (argc < 2)
    {
        printf("\n");
#if defined mdebug
        printf("\nSyntax: RefalABc_dbg source_file [option ... option]");
#else
        printf("\nSyntax: RefalABc source_file [option ... option]");
#endif
        printf("\nOptions:");
        printf("\n   -nn  No function names");
        printf("\n   -ns  No source listing");
        printf("\n   -fn  Full names");
        printf("\n   -as  Assembler module");
        printf("\n   -a,[options]  Assembler options");
        printf("\n\n");
        exit(1);
    };

    if (strlen(argv[1]) > MAX_PATHFILENAME - 4)
    {
        printf("\nSource file name too long\n");
        exit(1);
    }
    char parm[MAX_PATHFILENAME + 1];
    size_t i;
    strcpy(parm, argv[1]);
    strcat(parm, ".ref");
    printf("\n%s:\n", parm);
    sysin = fopen(parm, "r");
    if (sysin == NULL)
    {
        printf("Can't open %s\n", parm);
        exit(1);
    };
    terminal = stdout;
    refalab_source_listing = NULL;

    SET_time();

    options.source_listing = true;
    options.full_name = false;
    options.names = true;
    options.assembler_source_only = false;
    options.assembler_options[0] = '\0';
    options.assembler_options[8191] = '\0';
    for (size_t j = 2; j < (size_t)argc; ++j)
        if (strcmp(argv[j], "-nn") == 0)
            options.names = false;
        else if (strcmp(argv[j], "-ns") == 0)
            options.source_listing = false;
        else if (strcmp(argv[j], "-fn") == 0)
            options.full_name = true;
        else if (strcmp(argv[j], "-as") == 0)
            options.assembler_source_only = true;
        else if (strncmp(argv[j], "-a,", 3) == 0)
            strncpy(options.assembler_options, &argv[j][3], 8191);
        else
        {
            printf("Unknown option: %s\n", argv[j]);
            printf("Options may be: -nn, -ns, -fn, -as, -a,[options]\n");
            exit(1);
        }
    if (options.source_listing)
    {
        strcpy(parm, argv[1]);
        strcat(parm, ".lst");
        refalab_source_listing = fopen(parm, "w");
        if (refalab_source_listing == NULL)
        {
            printf("Can't open %s\n", parm);
            exit(8);
        }
    }
    strcpy(parm, argv[1]);
    strcat(parm, ".s");
    assembler_source = fopen(parm, "w");
    if (assembler_source == NULL)
    {
        printf("Can't open %s\n", parm);
        exit(8);
    }
    flags.was_error = false;
    cdnumb = 0;
    scanner.carriage_number = 0;
    bool impl = false;
    T_MODULE_STATES module_state = START_OF_MODULE;
    while (true)
        switch (module_state)
        {
        case START_OF_MODULE:
            kolosh = 0;
            scanner.module_number++;
            flags.was_cut = false;
            _eoj = false;
            card[CUT + 8] = '\n';
            prevlb[0] = '\0';
            mod_length = 0;
            memset(mod_name, '\0', MAX_IDENTIFIER_LENGTH + 1);
            for (i = 0; i < 7; ++i)
                sarr[i] = NULL;
            // "start" - directive work
            lblkey(false);
            if (_eoj)
            {
                module_state = END_OF_SYSIN;
                break;
            }
            s_init();
            if (strncasecmp(stmkey, "start", 5) != 0)
            {
                print_error_string("001 START-directive missing");
                scanner.module_name_length = 0;
                jstart();
                module_state = KEYS;
                break;
            }
            strncpy(mod_name, stmlbl, lbl_leng);
            strncpy(scanner.module_name, stmlbl, lbl_leng);
            scanner.module_name_length = lbl_leng;
            jstart();
            blout();
            if (m != CUT - 1 || c[m] != ' ')
                pch130();
            module_state = NEXT_STM;
            break;
        case NEXT_STM:
            // read of next sentence
            lblkey(false);
            module_state = KEYS;
            break;
        case KEYS:
            if (strncasecmp(stmkey, "impl", 4) == 0)
            {
                if (impl == true)
                    print_error_string("011 impl-directive in the impl-section");
                if (lbl_leng != 0)
                    pch130();
                else
                {
                    blout();
                    if (m != CUT - 1 || c[m] != ' ')
                        pch130();
                };
                impl = true;
            }
            else if (strncasecmp(stmkey, "l ", 2) == 0)
            {
                if (impl == false)
                    print_error_string("021 l-directive not in the impl-section");
                dir = true;
                trprev();
                compile_sentence(dir, stmlbl, lbl_leng);
            }
            else if (strncasecmp(stmkey, "r ", 2) == 0)
            {
                if (impl == false)
                    print_error_string("022 r-directive not in the impl-section");
                dir = false;
                trprev();
                compile_sentence(dir, stmlbl, lbl_leng);
            }
            else if (strncasecmp(stmkey, "start", 5) == 0)
            {
                if (impl == true)
                    print_error_string("012 start-directive in the impl-section");
                else
                    print_error_string("002 too many start-directive");
                blout();
                if (m != CUT - 1 || c[m] != ' ')
                    pch130();
            }
            else if (strncasecmp(stmkey, "end", 3) == 0)
            {
                if (prevlb[0] != '\0')
                    sempty(prevlb, strlen(prevlb));
                if (lbl_leng != 0)
                    pch130();
                else
                {
                    blout();
                    if (m != CUT - 1 || c[m] != ' ')
                        pch130();
                }
                module_state = END_STATEMENT;
                break;
            }
            else if (strncasecmp(stmkey, "entry", 5) == 0)
            {
                if (impl == true)
                    print_error_string("013 entry-directive in the impl-section");
                ilm(sentry);
            }
            else if (strncasecmp(stmkey, "extrn", 5) == 0)
            {
                if (impl == true)
                    print_error_string("014 extrn-directive in the impl-section");
                ilm(sextrn);
            }
            else if (strncasecmp(stmkey, "empty", 5) == 0)
            {
                if (impl == true)
                    print_error_string("015 empty-directive in the impl-section");
                il(sempty);
            }
            else if (strncasecmp(stmkey, "swap", 4) == 0)
            {
                if (impl == true)
                    print_error_string("016 swap-directive in the impl-section");
                il(sswap);
            }
            else if (strncasecmp(stmkey, "s ", 2) == 0)
            {
                if (impl == true)
                    print_error_string("017 s-directive in the impl-section");
                spdef(stmlbl, lbl_leng);
                specif(' ');
            }
            else if (strncasecmp(stmkey, "equ", 3) == 0)
            {
                if (impl == true)
                    print_error_string("018 equ-directive in the impl-section");
                equ();
            }
            else if (stmkey[0] == ' ')
            {
                trprev();
                if (lbl_leng != 0)
                {
                    if (impl == false)
                        print_error_string("023 function not in the impl-section");
                    strncpy(prevlb, stmlbl, lbl_leng);
                    prevlb[lbl_leng] = '\0';
                }
            }
            else
            {
                if (impl == false)
                    print_error_string("021 l-directive not in the impl-section");
                m = fixm; // return to left
                dir = true;
                trprev();
                compile_sentence(dir, stmlbl, lbl_leng);
            }
            if (!_eoj)
            {
                module_state = NEXT_STM;
                break;
            }
            module_state = END_IS_MISSING;
            break;
        case END_IS_MISSING:
            print_error_string("003 end-directive missing");
            kolosh++;
            module_state = END_STATEMENT;
            break;
        case END_STATEMENT:
            s_end();
            if (kolosh != 0)
            {
                flags.was_error = true;
                mod_length = 0;
            }
            else
            {
                jend();
                mod_length = jwhere();
            }
            s_term();
            pchzkl();
            module_state = END_OF_SYSIN;
            break;
        case END_OF_SYSIN:
            fclose(sysin);
            if (options.source_listing)
                fclose(refalab_source_listing);
            mod_length = jwhere();
            fclose(assembler_source);
            if (mod_length == 0 || flags.was_error)
                unlink(parm);
            else if (!options.assembler_source_only)
            {
                char cas[3 + 8191 + 1 + MAX_PATHFILENAME - 2 + 1];
                sprintf(cas, "as %s %s", options.assembler_options, parm);
                int res = system(cas);
#if defined POSIX
                if (WIFEXITED(res) != 0)
                    res = WEXITSTATUS(res);
                else
                    res = -1;
#endif
                if (res != 0)
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

static void trprev(void)
{ // perenos poslednej pustoj metki w tekuschuju
    size_t n = strlen(prevlb);
    if (n != 0 && lbl_leng == 0)
    {
        strncpy(stmlbl, prevlb, n);
        lbl_leng = n;
    }
    else if (n != 0)
        sempty(prevlb, n);
    prevlb[0] = '\0';
    return;
}

static void rdline(char *s)
{ // read CUT + 8 symbols from sysin
    empcard = true;
    size_t i;
    int cs = getc(sysin);
    for (i = 0; cs != '\n' && cs != EOF && i < CUT + 8; i++)
    {
        if (cs == '\t')
        {
            const size_t k = 8 - (i & 7);
            for (size_t j = 0; j < k; j++)
                *(s + i + j) = ' ';
            i += k - 1;
        }
        else if (cs < ' ' && cs > '\0')
            *(s + i) = ' ';
        else
        {
            *(s + i) = (char)cs;
            empcard = false;
        };
        cs = getc(sysin);
    }
    if (cs == EOF && i == 0)
        _eoj = true;
    for (; i < CUT + 8; i++)
        *(s + i) = ' ';
    return;
}

static void translate(const char *str, char *class1)
{ // L,D,* - classification procedure
    for (size_t i = 0; i < CUT; ++i)
    {
        *(class1 + i) = '*';
        const int j = *(str + i);
        if (j > 47)
            if (j < 58)
            {
                *(class1 + i) = 'D';
                continue;
            };
        if (j > 64)
            if (j < 91)
            {
                *(class1 + i) = 'L';
                continue;
            };
        if (j > 96)
            if (j < 123)
            {
                *(class1 + i) = 'L';
                continue;
            };
    }
    return;
}

static bool komm(void)
{
    const char *k;
    for (k = c; *k == ' ' || *k == '\t'; k++)
        ;
    if (*k == '*')
        return true;
    else
        return false;
}

static void rdcard(void)
{ // read card procedure
    while (true)
    {
        rdline(card);
        strncpy(c, card_cut, CUT);
        translate(card_cut, class);
        ++scanner.carriage_number;
        ++cdnumb;
        flags.was_card_print_file_source_listing = false;
        flags.was_card_print_terminal = false;
        if (options.source_listing)
            pchk();
        if (!flags.was_cut && komm())
            continue;
        if (empcard)
        {
            if (_eoj)
                return;
            continue;
        }
        break;
    }
    if (*(c + CUT - 1) != ' ')
        flags.was_cut = true;
    else
        flags.was_cut = false;
    if (*(c + CUT - 1) != ' ')
        *(c + CUT - 1) = '+'; //!!!
    m = 0;
    return;
}

//    directive label and keyword extraction
static void lblkey(bool pr)
{
    if (!pr)
        while (true)
        {
            rdcard();
            if (c[0] == ' ')
                lbl_leng = 0;
            else if (!get_id(stmlbl, &lbl_leng))
            {
                print_error_string("120 the first symbol is not letter or underscore or blank");
                continue;
            }
            break;
        }
    blout();
    memset(stmkey, ' ', 6);
    do
    {
        if (c[m] == ' ')
            break;
        fixm = m;
        size_t l = 0;
        while (c[m] != ' ')
        {
            if (m == CUT - 1)
            {
                const size_t delta = CUT - 1 - fixm;
                const int32_t fixm1 = (int32_t)(0 - delta);
                for (m = 0; m != delta; m++)
                {
                    c[fixm1 + (int32_t)m] = c[fixm + m];
                    class[fixm1 + (int32_t)m] = class[fixm + m];
                }
                rdcard();
                fixm = (size_t)fixm1;
                if (c[0] == ' ')
                    break;
            }
            if (l == 6)
            {
                m = fixm;
                stmkey[0] = 'l';
                break;
            }
            stmkey[l] = c[m];
            l++;
            m++;
        }
    } while (false);
    scn_station = false;
    left_part = true;
    return;
}

void scan_sentence_element(void)
{
    static char id[MAX_IDENTIFIER_LENGTH];
    static size_t id_leng;
    static const uint8_t *p;
    static size_t scode;
    current_sentence_element.code.tag = TAGO;
    current_sentence_element.code.info.codef = NULL;
    current_sentence_element.v_variable = false;
    current_sentence_element.specifier.tag = TAGO;
    current_sentence_element.specifier.info.codef = NULL;
    T_SCANNER_STATES scanner_state = STATE0;
    if (scn_station)
        scanner_state = STATE1;
    while (true)
        switch (scanner_state)
        {
        case STATE0:
            // among elements
            blout();
            switch (c[m])
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
            case ' ':
            case '\t':
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
                print_error_string_symbol("100 illegal symbol", c[m]);
                scanner_state = SCNERR;
            }
            break;
        case SCNERR:
            current_sentence_element.type = 0;
            scanner_state = SCNRET;
            break;
        case SCNSC:
            if (get_csmb(&current_sentence_element.code, id, &id_leng))
            {
                scanner_state = EGO;
                break;
            }
            scanner_state = SCNERR;
            break;
        case EGO:
            current_sentence_element.type = 1;
            scanner_state = SCNRET;
            break;
        case SCNL:
            current_sentence_element.type = 2;
            scanner_state = SCNGCR;
            break;
        case SCNR:
            current_sentence_element.type = 3;
            scanner_state = SCNGCR;
            break;
        case SCNS:
            current_sentence_element.type = 4;
            scanner_state = SCNV;
            break;
        case SCNW:
            current_sentence_element.type = 5;
            scanner_state = SCNV;
            break;
        case SCNVV:
            current_sentence_element.v_variable = true;
            scanner_state = SCNE;
            break;
        case SCNE:
            current_sentence_element.type = 6;
            scanner_state = SCNV;
            break;
        case SCNV:
            EH_ROMA;
            if (c[m] == '(')
            {
                EH_ROMA;
                if (left_part)
                {
                    current_sentence_element.specifier.info.codef = (uint8_t *)genlbl();
                    p = current_sentence_element.specifier.info.codef;
                    jlabel((T_U *)(void *)p);
                }
                if (specif(')'))
                {
                    EH_ROMA else
                    {
                        scanner_state = SCNERR;
                        break;
                    }
                }
            }
            else if (c[m] == ':')
            {
                EH_ROMA;
                if (!get_id(id, &id_leng))
                {
                    scanner_state = SOSH203;
                    break;
                }
                if (left_part)
                    current_sentence_element.specifier.info.codef = (uint8_t *)spref(id, id_leng, ')');
                if (c[m] == ':')
                {
                    EH_ROMA else
                    {
                        scanner_state = SOSH204;
                        break;
                    }
                }
            }
            scanner_state = SCNVD;
            break;
        case SCNVD:
            if (c[m] != '.')
            {
                scanner_state = OSH103;
                break;
            }
            EH_ROMA;
            scanner_state = SCNVI;
            break;
        case SCNVI:
            if (!get_id(id, &id_leng))
            {
                scanner_state = OSH102;
                break;
            }
            strncpy(current_sentence_element.identifier, id, id_leng);
            current_sentence_element.identifier_length = id_leng;
            scanner_state = SCNRET;
            break;
        case SCNKK:
            current_sentence_element.type = 7;
            if (c[m + 1] != ' ')
            {
                c[m - 1] = '&';
                size_t i;
                for (i = 1;
                     class[m + i] == 'L' || c[m + i] == '_' || class[m + i] == 'D';
                     i++)
                {
                    c[m + i - 1] = c[m + i];
                    class[m + i - 1] = class[m + i];
                }
                c[m + i - 1] = ' ';
                class[m + i - 1] = '*';
                m -= 2;
            }
            scanner_state = SCNGCR;
            break;
        case SCNP:
            current_sentence_element.type = 8;
            scanner_state = SCNGCR;
            break;
        case SCNEOL:
            current_sentence_element.type = 9;
            left_part = false;
            scanner_state = SCNGCR;
            break;
        case SCNEOS:
            current_sentence_element.type = 10;
            scanner_state = SCNRET;
            break;
        case SCNA:
            EH_ROMA;
            if (m == CUT - 1)
            {
                scanner_state = OSH101;
                break;
            }
            if (c[m] == '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            scn_station = true;
            scanner_state = SCNCHR;
            break;
        case STATE1: // within letter chain
            if (m == CUT - 1)
            {
                scanner_state = OSH101;
                break;
            }
            if (c[m] != '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            EH_ROMA;
            if (c[m] == '\'')
            {
                scanner_state = SCNCHR;
                break;
            }
            scn_station = false;
            scanner_state = STATE0;
            break;
        case SCNCHR:
            current_sentence_element.code.tag = TAGO;
            current_sentence_element.code.info.codef = NULL;
            if (c[m] == '\\')
                // control symbols
                switch (c[++m])
                {
                case '\\':
                    break;
                case 'n':
                    c[m] = '\012';
                    break;
                case 't':
                    c[m] = '\011';
                    break;
                case 'v':
                    c[m] = '\013';
                    break;
                case 'r':
                    c[m] = '\015';
                    break;
                case 'f':
                    c[m] = '\014';
                    break;
                case '0':
                    if (c[m + 1] >= '0' && c[m + 1] <= '7')
                    {
                        uint32_t j = 0;
                        for (size_t i = 1; i < 3; i++)
                            if (c[m + i] >= '0' && c[m + i] <= '7')
                                j = j * 8 + (uint32_t)(c[m + i] - '0');
                            else
                            {
                                m--;
                                break;
                            }
                        m += 2;
                        c[m] = (char)(j & 255);
                    }
                    else
                        c[m] = '\0';
                    break;
                default:
                    if (c[m] >= '0' && c[m] <= '7')
                    {
                        uint32_t j = 0;
                        for (size_t i = 0; i < 3; i++)
                            if (c[m + i] >= '0' && c[m + i] <= '7')
                                j = j * 8 + (uint32_t)(c[m + i] - '0');
                            else
                            {
                                m--;
                                break;
                            }
                        m += 2;
                        c[m] = (char)(j & 255);
                    }
                    else
                        m--;
                }
            scanner_state = PROD;
            break;
        case PROD:
            current_sentence_element.code.info.infoc = c[m];
            current_sentence_element.type = 1;
            scanner_state = SCNGCR;
            break;
        case FSCN:
            scode = 0;
            scanner_state = SABBR;
            break;
        case NSCN:
            scode = 1;
            scanner_state = SABBR;
            break;
        case RSCN:
            scode = 2;
            scanner_state = SABBR;
            break;
        case OSCN:
            scode = 3;
            scanner_state = SABBR;
            break;
        case DSCN:
            scode = 4;
            scanner_state = SABBR;
            break;
        case LSCN:
            scode = 5;
            scanner_state = SABBR;
            break;
        case SABBR:
            current_sentence_element.type = 4;
            if (left_part)
            {
                if (*(sarr + scode) == NULL)
                {
                    *(sarr + scode) = (char *)(void *)genlbl();
                    jlabel((T_U *)(void *)*(sarr + scode));
                    gsp((uint8_t)(scode + 7));
                    gsp(ns_ngw);
                };
                current_sentence_element.specifier.info.codef = (uint8_t *)*(sarr + scode);
            };
            EH_ROMA;
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
            EH_ROMA;
            scanner_state = SCNRET;
            break;
        case SCNRET:
            return;
        }
}

static void gsp(uint8_t n)
{
    if (left_part)
        jbyte(n);
    return;
}

static bool specif(char tail)
{ // specifier compiler
    bool neg = false;
    char id[MAX_IDENTIFIER_LENGTH];
    size_t lid;
    T_LINKTI code;
    T_SPECIFIER_STATES specifier_state = SPCBLO;
    while (true)
        switch (specifier_state)
        {
        case SPCBLO:
            blout();
            specifier_state = SPCPRC;
            break;
        case SPCPRC:
            switch (c[m])
            {
            case ' ':
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
                print_error_string_symbol("201 within specifier invalid symbol ", c[m]);
                specifier_state = OSH200;
            }
            break;
        case SPCFF:
            gsp(ns_ngw);
            if (neg)
                print_error_string("207 within specifier default ')' ");
            if (tail == ')')
            {
                specifier_state = OSH206;
                break;
            }
            return true;
        case SPCL:
            if (neg)
            {
                specifier_state = OSH202;
                break;
            }
            neg = true;
            gsp(ns_ng);
            specifier_state = SPCGC;
            break;
        case SPCR:
            if (!neg)
            {
                specifier_state = SPCR1;
                break;
            }
            EH_ROMA0;
            blout();
            if (c[m] == '(')
            {
                specifier_state = SPCGC;
                break;
            }
            if (c[m] == ')')
            {
                specifier_state = SPCR1;
                break;
            }
            if (c[m] == ' ')
            {
                specifier_state = SPCR2;
                break;
            }
            neg = false;
            gsp(ns_ng);
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
            gsp(ns_ngw);
            return true;
        case SPCESC:
            if (!get_csmb(&code, id, &lid))
            {
                specifier_state = OSH200;
                break;
            }
            gsp(ns_sc);
            if (left_part)
                generate_symbol(&code);
            specifier_state = SPCBLO;
            break;
        case SPCSP:
            EH_ROMA0;
            if (!get_id(id, &lid))
            {
                specifier_state = OSH203;
                break;
            }
            if (strncmp(stmlbl, id, lid) == 0 && (lid == MAX_IDENTIFIER_LENGTH || stmlbl[lid] == ' '))
                print_error_string("209 specifier is defined through itself");
            T_U *p = spref(id, lid, tail);
            gsp(ns_cll);
            if (left_part)
                j3addr(p);
            if (c[m] == ':')
            {
                specifier_state = SPCGC;
                break;
            }
            specifier_state = OSH204;
            break;
        case SPCA:
            EH_ROMA0;
            if (m == CUT - 1)
            {
                specifier_state = OSH205;
                break;
            }
            if (c[m] != '\'')
            {
                specifier_state = SPCA1;
                break;
            }
            gsp(ns_sc);
            if (left_part)
            {
                code.tag = TAGO;
                code.info.codef = NULL;
                code.info.infoc = '\'';
                generate_symbol(&code);
            }
            specifier_state = SPCGC;
            break;
        case SPCA1:
            gsp(ns_sc);
            if (left_part)
            {
                if (c[m] == '\\')
                    // control symbols ---------------
                    switch (c[++m])
                    {
                    case '\\':
                        break;
                    case 'n':
                        c[m] = '\012';
                        break;
                    case 't':
                        c[m] = '\011';
                        break;
                    case 'v':
                        c[m] = '\013';
                        break;
                    case 'r':
                        c[m] = '\015';
                        break;
                    case 'f':
                        c[m] = '\014';
                        break;
                    case '0':
                        if (c[m + 1] >= '0' && c[m + 1] <= '7')
                        {
                            uint32_t j = 0;
                            for (size_t i = 1; i < 3; i++)
                                if (c[m + i] >= '0' && c[m + i] <= '7')
                                    j = j * 8 + (uint32_t)(c[m + i] - '0');
                                else
                                {
                                    m--;
                                    break;
                                }
                            m += 2;
                            c[m] = (char)(j & 255);
                        }
                        else
                            c[m] = '\0';
                        break;
                    default:
                        if (c[m] >= '0' && c[m] <= '7')
                        {
                            uint32_t j = 0;
                            for (size_t i = 0; i < 3; i++)
                                if (c[m + i] >= '0' && c[m + i] <= '7')
                                    j = j * 8 + (uint32_t)(c[m + i] - '0');
                                else
                                {
                                    m--;
                                    break;
                                }
                            m += 2;
                            c[m] = (char)(j & 255);
                        }
                        else
                            m--;
                    }
                code.tag = TAGO;
                code.info.codef = NULL;
                code.info.infoc = c[m];
                generate_symbol(&code);
            }
            EH_ROMA0;
            if (m == CUT - 1)
            {
                specifier_state = OSH205;
                break;
            }
            if (c[m] != '\'')
            {
                specifier_state = SPCA1;
                break;
            }
            EH_ROMA0;
            if (c[m] == '\'')
            {
                specifier_state = SPCA1;
                break;
            }
            specifier_state = SPCBLO;
            break;
        case SPCES:
            gsp(ns_s);
            specifier_state = SPCGC;
            break;
        case SPCEB:
            gsp(ns_b);
            specifier_state = SPCGC;
            break;
        case SPCEW:
            gsp(ns_w);
            specifier_state = SPCGC;
            break;
        case SPCEF:
            gsp(ns_f);
            specifier_state = SPCGC;
            break;
        case SPCEN:
            gsp(ns_n);
            specifier_state = SPCGC;
            break;
        case SPCER:
            gsp(ns_r);
            specifier_state = SPCGC;
            break;
        case SPCEO:
            gsp(ns_o);
            specifier_state = SPCGC;
            break;
        case SPCEL:
            gsp(ns_l);
            specifier_state = SPCGC;
            break;
        case SPCED:
            gsp(ns_d);
            specifier_state = SPCGC;
            break;
        case SPCGC:
            EH_ROMA0;
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

static void pchk(void)
{ // writing of card into file source listing
    if (!flags.was_card_print_file_source_listing && refalab_source_listing != NULL)
    {
        flags.was_card_print_file_source_listing = true;
        card[CUT] = '\0';
        if (!_eoj)
        {
            char tmpstr[CUT + 28];
            sprintf(tmpstr, "%4d %s", cdnumb, card);
            size_t i;
            for (i = CUT + 4; i > 4; i--)
                if (tmpstr[i] != ' ')
                    break;
            i++;
            tmpstr[i] = '\n';
            i++;
            tmpstr[i] = '\0';
            fputs(tmpstr, refalab_source_listing);
        }
    }
    return;
}

static void pchk_t(void)
{ // card writing into terminal
    if (!flags.was_card_print_terminal)
    {
        flags.was_card_print_terminal = true;
        card[CUT] = '\0';
        if (!_eoj)
        {
            char tmpstr[CUT + 28];
            sprintf(tmpstr, "%4d %s\n", cdnumb, card);
            fputs(tmpstr, terminal);
        }
    }
    return;
}

static void il(void (*prog)(const char *, size_t)) // treatment of directives having 'EMPTY' type
{
    if (lbl_leng != 0)
    {
        pch130();
        return;
    }
    blout();
    while (true)
    {
        char id[MAX_IDENTIFIER_LENGTH];
        size_t lid;
        if (!get_id(id, &lid))
            break;
        (*prog)(id, lid);
        blout();
        if (m == CUT - 1 && c[m] == ' ')
            return;
        if (c[m] == ',')
        {
            EH_ROMA;
            if (c[m] == ' ')
                blout();
            continue;
        }
        break;
    }
    pch130();
    return;
}

static void ilm(void (*prog)(const char *, size_t, const char *, size_t)) // treatment of directives having 'ENTRY' type
{
    if (lbl_leng != 0)
    {
        pch130();
        return;
    }
    blout();
    while (true)
    {
        char id[MAX_IDENTIFIER_LENGTH];
        size_t lid;
        if (!get_id(id, &lid))
            break;
        char ide[MAX_EXTERN_IDENTIFIER_LENGTH];
        size_t lide;
        if (c[m] == '(')
        {
            EH_ROMA;
            if (!get_idm(ide, &lide))
                break;
            (*prog)(id, lid, ide, lide);
            if (c[m] != ')')
                break;
            EH_ROMA;
        }
        else
        {
            lide = lid > MAX_EXTERN_IDENTIFIER_LENGTH ? MAX_EXTERN_IDENTIFIER_LENGTH : lid;
            strncpy(ide, id, lide);
            (*prog)(id, lid, ide, lide);
        }
        blout();
        if (m == CUT - 1 && c[m] == ' ')
            return;
        if (c[m] == ',')
        {
            EH_ROMA;
            if (c[m] == ' ')
                blout();
            continue;
        }
        break;
    }
    pch130();
    return;
}

static void equ(void)
{ // treatement of directives having 'EQU' type
    blout();
    char id[MAX_IDENTIFIER_LENGTH];
    size_t lid;
    do
    {
        if (!get_id(id, &lid))
            break;
        sequ(stmlbl, lbl_leng, id, lid);
        blout();
        if (m == CUT - 1 && c[m] == ' ')
            return;
    } while (false);
    pch130();
    return;
}

static void pch130(void)
{
    print_error_string("130 invalid record format");
    return;
}

static bool get_csmb(T_LINKTI *code, char id[MAX_IDENTIFIER_LENGTH], size_t *lid) // procedure read multiple symbol
{
    code->tag = TAGO;
    code->info.codef = NULL;
    do
    {
        if (class[m] == 'D')
        {
            code->tag = TAGN;
            code->info.codef = NULL;
            code->info.coden = 0;
            int64_t k = c[m] - '0';
            bool csmbend = false;
            while (true)
            {
                EH_ROMA0;
                if (class[m] != 'D')
                {
                    code->tag = TAGN;
                    code->info.codef = NULL;
                    code->info.coden = (uint32_t)k;
                    csmbend = true;
                    break;
                }
                const int64_t l = c[m] - '0';
                k = k * 10 + l;
                if (k <= MAX_NUMBER)
                    continue;
                break;
            }
            if (csmbend)
                break;
            while (true)
            {
                EH_ROMA0;
                if (class[m] == 'D')
                    continue;
                break;
            }
            char osh111[64];
            sprintf(osh111, "111 symbol-number > %lld", MAX_NUMBER);
            print_error_string(osh111);
            break;
        }
        EH_ROMA0;
        if (!get_id(id, lid))
        {
            print_error_string("112 unknown type of the multiple symbol");
            return false;
        }
        code->info.codef = (uint8_t *)fnref(id, *lid);
        code->tag = TAGF;
    } while (false);
    return true;
}

static bool get_id(char id[MAX_IDENTIFIER_LENGTH], size_t *lid)
{ // read identifier
    memset(id, ' ', MAX_IDENTIFIER_LENGTH);
    if (class[m] != 'L' && c[m] != '_')
        return false;
    id[0] = (char)toupper(c[m]);
    for (*lid = 1; *lid < MAX_IDENTIFIER_LENGTH; (*lid)++)
    {
        EH_ROMA0;
        if (class[m] != 'L' && c[m] != '_' && class[m] != 'D')
            return true;
        id[*lid] = (char)toupper(c[m]);
    }
    size_t i = 0;
    while (class[m] == 'L' || class[m] == 'D' || c[m] == '_')
    {
        EH_ROMA0;
        i++;
    }
    if (i > 1)
    {
        char osh113[64];
        sprintf(osh113, "113 identifier length > %d", MAX_IDENTIFIER_LENGTH);
        print_error_string(osh113);
    }
    return true;
}

// read external identifier
static bool get_idm(char id[MAX_EXTERN_IDENTIFIER_LENGTH], size_t *lid)
{
    if (class[m] != 'L' && c[m] != '_')
        return false;
    id[0] = (char)toupper(c[m]);
    for (*lid = 1; *lid < MAX_EXTERN_IDENTIFIER_LENGTH; (*lid)++)
    {
        EH_ROMA0;
        if (class[m] != 'L' && c[m] != '_' && class[m] != 'D')
            return true;
        id[*lid] = (char)toupper(c[m]);
    }
    size_t i = 0;
    while (class[m] == 'L' || class[m] == 'D' || c[m] == '_')
    {
        EH_ROMA0;
        i++;
    }
    if (i > 1)
    {
        char osh114[64];
        sprintf(osh114, "114 external identifier length > %d", MAX_EXTERN_IDENTIFIER_LENGTH);
        print_error_string(osh114);
    }
    return true;
}

//********************************************************************
//                  missing blanks
//       before call: (m = CUT - 1) !! (m != CUT - 1)
//  under call:((m=CUT - 1)&&(c[m]=' '))!!((m!=CUT - 1)&&(c[m]!=' '))
//********************************************************************
static void blout(void)
{
    while (true)
    {
        while (m != CUT - 1 && c[m] == ' ')
            m++;
        if (c[m] == '+')
        {
            rdcard();
            if (_eoj)
                return;
            continue;
        }
        return;
    }
}

static void pchzkl(void)
{ // print conclusion
    char pr_line[180];
    sprintf(pr_line,
            "mod_name = %-40s    mod_length(lines) = %d\n", mod_name, cdnumb);
    if (options.source_listing)
        fputs(pr_line, refalab_source_listing);
    fputs(pr_line, terminal);
    cdnumb = 0;
    if (kolosh != 0)
        sprintf(pr_line,
                "errors   = %-3d         obj_length(bytes) = %zu\n", kolosh, mod_length);
    else
        sprintf(pr_line,
                "                       obj_length(bytes) = %zu\n", mod_length);
    if (options.source_listing)
        fputs(pr_line, refalab_source_listing);
    fputs(pr_line, terminal);
    GET_time();
    return;
}

void processing_error(void)
{
    pchk();
    pchk_t();
    kolosh++;
    return;
}

//----------  end of file REFAL.C  -------------------
