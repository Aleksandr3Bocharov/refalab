// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-23
// https://github.com/Aleksandr3Bocharov/RefalAB

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

typedef enum mod_states
{
    START_OF_MODULE,
    NEXT_STM,
    KEYS,
    END_IS_MISSING,
    END_STATEMENT,
    END_OF_SYSIN
} T_MOD_STATES;

typedef enum scn_states
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
} T_SCN_STATES;

typedef enum sp_states
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
} T_SP_STATES;

T_OPT options;

T_SCN scn_;

T_SCN_E scn_e;

static struct
{
    bool was_cut;
    bool uzhe_krt;
    bool was_err;
    bool uzhekrt_t;
} flags;

FILE *sysprint, *systerm;
FILE *syslin; // for assem
FILE *systxt; // for module names

uint32_t nommod;

// Aleksandr Bocharov   // compiler version
static const char *vers_i = "RefalAB Version 0.9.0 20250917 (c) Aleksandr Bocharov";

static FILE *sysin;
static size_t m;           // current symbol number
static bool empcard;       // flags for empty card
static char card[CUT + 9]; // card buffer (input)
static const char *card_cut = card;
static uint32_t cdnumb; // card number
static bool dir;        // L,R - flag
static uint32_t kolosh;
static const char ns_b = '\6';
static const char ns_cll = '\0';
static const char ns_d = '\13';
static const char ns_f = '\7';
static const char ns_l = '\14';
static const char ns_n = '\10';
static const char ns_ng = '\2';
static const char ns_ngw = '\3';
static const char ns_o = '\12';
static const char ns_r = '\11';
static const char ns_s = '\5';
static const char ns_sc = '\4';
static const char ns_w = '\1';
static char strg_c[CUT + 6];
static char *c = strg_c + 6;
static char class_cut[CUT + 6];
static char *class = class_cut + 6;
static bool scn_station; // scanner station - in(1),out(0) literal chain
static bool left_part;
static char *sarr[7]; // abbreviated specifier table
static char stmlbl[MAX_ID_LEN];
static size_t lbl_leng;
static char prevlb[MAX_ID_LEN + 1];
static char stmkey[6];
static size_t fixm;                   // start sentence position
static char mod_name[MAX_ID_LEN + 1]; // module name
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
static void gsp(char n);
static bool specif(char tail);
static bool get_id(char id[MAX_ID_LEN], size_t *lid);
static bool get_idm(char id[MAX_EXT_ID_LEN], size_t *lid);
static bool get_csmb(T_LINKTI *code, char id[MAX_ID_LEN], size_t *lid);

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
    int64_t is = (int64_t)difftime(t1.tv_sec, t0.tv_sec);
    if (in < 0)
    {
        in += 1000000000;
        is--;
    }
    int64_t im = is / 60;
    is %= 60;
    char s[64];
    sprintf(s, "%02lld:%02lld.%09ld", im, is, in);
    printf("                       elapsed time      = %s\n", s);
    return;
}

int main(int argc, char *argv[])
{
    systerm = NULL;
    nommod = 0;
    printf("\n");
    printf("%s", vers_i);
    if (argc < 2)
    {
        printf("\n");
#ifdef mdebug
        printf("\nSyntax: RefalAB_dbg source_file [option ... option]");
#else
        printf("\nSyntax: RefalAB source_file [option ... option]");
#endif
        printf("\nOptions:");
        printf("\n   nn  No function names");
        printf("\n   ns  No source listing");
        printf("\n   fn  Full names");
        printf("\n\n");
        exit(1);
    };

    char *parm = (char *)malloc(strlen(argv[1]) + 5);
    if (parm == NULL)
    {
        printf("\nNo enough memory for initialization\n");
        exit(1);
    }
#ifdef mdebug
    fprintf(stderr, "\nmalloc(main): parm=%p", (void *)parm);
#endif
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
    systerm = stdout;
    sysprint = NULL;

    SET_time();

    options.source = true;
    options.extname = false;
    options.names = true;
    for (size_t j = 2; j < (size_t)argc; ++j)
        if (strcmp(argv[j], "nn") == 0)
            options.names = false;
        else if (strcmp(argv[j], "ns") == 0)
            options.source = false;
        else if (strcmp(argv[j], "fn") == 0)
            options.extname = true;
        else
        {
            printf("Unknown option: %s\n", argv[j]);
            printf("Options may be: nn, ns, fn\n");
            exit(1);
        }
    if (options.source)
    {
        strcpy(parm, argv[1]);
        strcat(parm, ".lst");
        sysprint = fopen(parm, "w");
        if (sysprint == NULL)
        {
            printf("Can't open %s\n", parm);
            exit(8);
        }
    }
    strcpy(parm, argv[1]);
    strcat(parm, ".s");
    syslin = fopen(parm, "w");
    if (syslin == NULL)
    {
        printf("Can't open %s\n", parm);
        exit(8);
    }
    flags.was_err = false;
    cdnumb = 0;
    scn_.nomkar = 0;
    bool impl = false;
    T_MOD_STATES mod_state = START_OF_MODULE;
    while (true)
        switch (mod_state)
        {
        case START_OF_MODULE:
            kolosh = 0;
            nommod++;
            flags.was_cut = false;
            _eoj = false;
            card[CUT + 8] = '\n';
            prevlb[0] = '\0';
            mod_length = 0;
            memset(mod_name, '\0', MAX_ID_LEN + 1);
            for (i = 0; i < 7; ++i)
                sarr[i] = NULL;
            // "start" - directive work
            lblkey(false);
            if (_eoj)
            {
                mod_state = END_OF_SYSIN;
                break;
            }
            s_init();
            if (strncasecmp(stmkey, "start", 5) != 0)
            {
                pchosh("001 START-directive missing");
                scn_.modnmlen = 0;
                jstart();
                mod_state = KEYS;
                break;
            }
            strncpy(mod_name, stmlbl, lbl_leng);
            strncpy(scn_.modname_var, stmlbl, lbl_leng);
            scn_.modnmlen = lbl_leng;
            jstart();
            blout();
            if (m != CUT - 1 || c[m] != ' ')
                pch130();
            mod_state = NEXT_STM;
            break;
        case NEXT_STM:
            // read of next sentence
            lblkey(false);
            mod_state = KEYS;
            break;
        case KEYS:
            if (strncasecmp(stmkey, "impl", 4) == 0)
            {
                if (impl == true)
                    pchosh("011 impl-directive in the impl-section");
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
                    pchosh("021 l-directive not in the impl-section");
                dir = true;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            else if (strncasecmp(stmkey, "r ", 2) == 0)
            {
                if (impl == false)
                    pchosh("022 r-directive not in the impl-section");
                dir = false;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            else if (strncasecmp(stmkey, "start", 5) == 0)
            {
                if (impl == true)
                    pchosh("012 start-directive in the impl-section");
                else
                    pchosh("002 too many start-directive");
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
                mod_state = END_STATEMENT;
                break;
            }
            else if (strncasecmp(stmkey, "entry", 5) == 0)
            {
                if (impl == true)
                    pchosh("013 entry-directive in the impl-section");
                ilm(sentry);
            }
            else if (strncasecmp(stmkey, "extrn", 5) == 0)
            {
                if (impl == true)
                    pchosh("014 extrn-directive in the impl-section");
                ilm(sextrn);
            }
            else if (strncasecmp(stmkey, "empty", 5) == 0)
            {
                if (impl == true)
                    pchosh("015 empty-directive in the impl-section");
                il(sempty);
            }
            else if (strncasecmp(stmkey, "swap", 4) == 0)
            {
                if (impl == true)
                    pchosh("016 swap-directive in the impl-section");
                il(sswap);
            }
            else if (strncasecmp(stmkey, "s ", 2) == 0)
            {
                if (impl == true)
                    pchosh("017 s-directive in the impl-section");
                spdef(stmlbl, lbl_leng);
                specif(' ');
            }
            else if (strncasecmp(stmkey, "equ", 3) == 0)
            {
                if (impl == true)
                    pchosh("018 equ-directive in the impl-section");
                equ();
            }
            else if (stmkey[0] == ' ')
            {
                trprev();
                if (lbl_leng != 0)
                {
                    if (impl == false)
                        pchosh("023 function not in the impl-section");
                    strncpy(prevlb, stmlbl, lbl_leng);
                    prevlb[lbl_leng] = '\0';
                }
            }
            else
            {
                if (impl == false)
                    pchosh("021 l-directive not in the impl-section");
                m = fixm; // return to left
                dir = true;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            if (!_eoj)
            {
                mod_state = NEXT_STM;
                break;
            }
            mod_state = END_IS_MISSING;
            break;
        case END_IS_MISSING:
            pchosh("003 end-directive missing");
            kolosh++;
            mod_state = END_STATEMENT;
            break;
        case END_STATEMENT:
            s_end();
            if (kolosh != 0)
            {
                flags.was_err = true;
                mod_length = 0;
            }
            else
            {
                jend();
                mod_length = jwhere();
            }
            s_term();
            pchzkl();
            mod_state = END_OF_SYSIN;
            break;
        case END_OF_SYSIN:
            fclose(sysin);
            if (options.source)
                fclose(sysprint);
            mod_length = jwhere();
            fclose(syslin);
            if (mod_length == 0 || flags.was_err)
                unlink(parm);
#ifdef mdebug
            fprintf(stderr, "free(main) parm=%p\n", (void *)parm);
#endif
            free(parm);
            parm = NULL;
            if (flags.was_err)
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
        ++scn_.nomkar;
        ++cdnumb;
        flags.uzhe_krt = false;
        flags.uzhekrt_t = false;
        if (options.source)
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
                pchosh("120 the first symbol is not letter or underscore or blank");
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

void scan(void)
{
    static char id[MAX_ID_LEN];
    static size_t id_leng;
    static const uint8_t *p;
    static size_t scode;
    scn_e.code.tag = TAGO;
    scn_e.code.info.codef = NULL;
    scn_e.v = false;
    scn_e.spec.tag = TAGO;
    scn_e.spec.info.codef = NULL;
    T_SCN_STATES scn_state = STATE0;
    if (scn_station)
        scn_state = STATE1;
    while (true)
        switch (scn_state)
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
                scn_state = SCNSC;
                break;
            case '(':
                scn_state = SCNL;
                break;
            case ')':
                scn_state = SCNR;
                break;
            case 's':
            case 'S':
                scn_state = SCNS;
                break;
            case 'w':
            case 'W':
                scn_state = SCNW;
                break;
            case 'v':
            case 'V':
                scn_state = SCNVV;
                break;
            case 'e':
            case 'E':
                scn_state = SCNE;
                break;
            case '<':
                scn_state = SCNKK;
                break;
            case '>':
                scn_state = SCNP;
                break;
            case '=':
                scn_state = SCNEOL;
                break;
            case ' ':
            case '\t':
                scn_state = SCNEOS;
                break;
            case '\'':
                scn_state = SCNA;
                break;
            case 'f':
            case 'F':
                scn_state = FSCN;
                break;
            case 'n':
            case 'N':
                scn_state = NSCN;
                break;
            case 'r':
            case 'R':
                scn_state = RSCN;
                break;
            case 'o':
            case 'O':
                scn_state = OSCN;
                break;
            case 'd':
            case 'D':
                scn_state = DSCN;
                break;
            case 'l':
            case 'L':
                scn_state = LSCN;
                break;
            default:
                pchosa("100 illegal symbol", c[m]);
                scn_state = SCNERR;
            }
            break;
        case SCNERR:
            scn_e.t = 0;
            scn_state = SCNRET;
            break;
        case SCNSC:
            if (get_csmb(&scn_e.code, id, &id_leng))
            {
                scn_state = EGO;
                break;
            }
            scn_state = SCNERR;
            break;
        case EGO:
            scn_e.t = 1;
            scn_state = SCNRET;
            break;
        case SCNL:
            scn_e.t = 2;
            scn_state = SCNGCR;
            break;
        case SCNR:
            scn_e.t = 3;
            scn_state = SCNGCR;
            break;
        case SCNS:
            scn_e.t = 4;
            scn_state = SCNV;
            break;
        case SCNW:
            scn_e.t = 5;
            scn_state = SCNV;
            break;
        case SCNVV:
            scn_e.v = true;
            scn_state = SCNE;
            break;
        case SCNE:
            scn_e.t = 6;
            scn_state = SCNV;
            break;
        case SCNV:
            EH_ROMA;
            if (c[m] == '(')
            {
                EH_ROMA;
                if (left_part)
                {
                    scn_e.spec.info.codef = (uint8_t *)genlbl();
                    p = scn_e.spec.info.codef;
                    jlabel((T_U *)(void *)p);
                }
                if (specif(')'))
                {
                    EH_ROMA else
                    {
                        scn_state = SCNERR;
                        break;
                    }
                }
            }
            else if (c[m] == ':')
            {
                EH_ROMA;
                if (!get_id(id, &id_leng))
                {
                    scn_state = SOSH203;
                    break;
                }
                if (left_part)
                    scn_e.spec.info.codef = (uint8_t *)spref(id, id_leng, ')');
                if (c[m] == ':')
                {
                    EH_ROMA else
                    {
                        scn_state = SOSH204;
                        break;
                    }
                }
            }
            scn_state = SCNVD;
            break;
        case SCNVD:
            if (c[m] != '.')
            {
                scn_state = OSH103;
                break;
            }
            EH_ROMA;
            scn_state = SCNVI;
            break;
        case SCNVI:
            if (!get_id(id, &id_leng))
            {
                scn_state = OSH102;
                break;
            }
            strncpy(scn_e.si, id, id_leng);
            scn_e.si_leng = id_leng;
            scn_state = SCNRET;
            break;
        case SCNKK:
            scn_e.t = 7;
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
            scn_state = SCNGCR;
            break;
        case SCNP:
            scn_e.t = 8;
            scn_state = SCNGCR;
            break;
        case SCNEOL:
            scn_e.t = 9;
            left_part = false;
            scn_state = SCNGCR;
            break;
        case SCNEOS:
            scn_e.t = 10;
            scn_state = SCNRET;
            break;
        case SCNA:
            EH_ROMA;
            if (m == CUT - 1)
            {
                scn_state = OSH101;
                break;
            }
            if (c[m] == '\'')
            {
                scn_state = SCNCHR;
                break;
            }
            scn_station = true;
            scn_state = SCNCHR;
            break;
        case STATE1: // within letter chain
            if (m == CUT - 1)
            {
                scn_state = OSH101;
                break;
            }
            if (c[m] != '\'')
            {
                scn_state = SCNCHR;
                break;
            }
            EH_ROMA;
            if (c[m] == '\'')
            {
                scn_state = SCNCHR;
                break;
            }
            scn_station = false;
            scn_state = STATE0;
            break;
        case SCNCHR:
            scn_e.code.tag = TAGO;
            scn_e.code.info.codef = NULL;
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
            scn_state = PROD;
            break;
        case PROD:
            scn_e.code.info.infoc[0] = c[m];
            scn_e.t = 1;
            scn_state = SCNGCR;
            break;
        case FSCN:
            scode = 0;
            scn_state = SABBR;
            break;
        case NSCN:
            scode = 1;
            scn_state = SABBR;
            break;
        case RSCN:
            scode = 2;
            scn_state = SABBR;
            break;
        case OSCN:
            scode = 3;
            scn_state = SABBR;
            break;
        case DSCN:
            scode = 4;
            scn_state = SABBR;
            break;
        case LSCN:
            scode = 5;
            scn_state = SABBR;
            break;
        case SABBR:
            scn_e.t = 4;
            if (left_part)
            {
                if (*(sarr + scode) == NULL)
                {
                    *(sarr + scode) = (char *)(void *)genlbl();
                    jlabel((T_U *)(void *)*(sarr + scode));
                    gsp((char)(scode + 7));
                    gsp(ns_ngw);
                };
                scn_e.spec.info.codef = (uint8_t *)*(sarr + scode);
            };
            EH_ROMA;
            scn_state = SCNVD;
            break;
        case OSH101:
            pchosh("101 default of left apostroph");
            scn_state = SCNERR;
            break;
        case OSH102:
            pchosh("102 sign '.' followed by no letter or underscore");
            scn_state = SCNERR;
            break;
        case OSH103:
            pchosh("103 sign '.' expected");
            scn_state = SCNERR;
            break;
        case SOSH203:
            pchosh("203 sign ':' followed by no letter or underscore");
            scn_state = SCNERR;
            break;
        case SOSH204:
            pchosh("204 default last ':' within specifier");
            scn_state = SCNERR;
            break;
        case SCNGCR:
            EH_ROMA;
            scn_state = SCNRET;
            break;
        case SCNRET:
            return;
        }
}

static void gsp(char n)
{
    if (left_part)
        jbyte(n);
    return;
}

static bool specif(char tail)
{ // specifier compiler
    bool neg = false;
    char id[MAX_ID_LEN];
    size_t lid;
    T_LINKTI code;
    T_SP_STATES sp_state = SPCBLO;
    while (true)
        switch (sp_state)
        {
        case SPCBLO:
            blout();
            sp_state = SPCPRC;
            break;
        case SPCPRC:
            switch (c[m])
            {
            case ' ':
                sp_state = SPCFF;
                break;
            case '(':
                sp_state = SPCL;
                break;
            case ')':
                sp_state = SPCR;
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
                sp_state = SPCESC;
                break;
            case ':':
                sp_state = SPCSP;
                break;
            case '\'':
                sp_state = SPCA;
                break;
            case 's':
            case 'S':
                sp_state = SPCES;
                break;
            case 'b':
            case 'B':
                sp_state = SPCEB;
                break;
            case 'w':
            case 'W':
                sp_state = SPCEW;
                break;
            case 'f':
            case 'F':
                sp_state = SPCEF;
                break;
            case 'n':
            case 'N':
                sp_state = SPCEN;
                break;
            case 'r':
            case 'R':
                sp_state = SPCER;
                break;
            case 'o':
            case 'O':
                sp_state = SPCEO;
                break;
            case 'l':
            case 'L':
                sp_state = SPCEL;
                break;
            case 'd':
            case 'D':
                sp_state = SPCED;
                break;
            default:
                pchosa("201 within specifier invalid symbol ", c[m]);
                sp_state = OSH200;
            }
            break;
        case SPCFF:
            gsp(ns_ngw);
            if (neg)
                pchosh("207 within specifier default ')' ");
            if (tail == ')')
            {
                sp_state = OSH206;
                break;
            }
            return true;
        case SPCL:
            if (neg)
            {
                sp_state = OSH202;
                break;
            }
            neg = true;
            gsp(ns_ng);
            sp_state = SPCGC;
            break;
        case SPCR:
            if (!neg)
            {
                sp_state = SPCR1;
                break;
            }
            EH_ROMA0;
            blout();
            if (c[m] == '(')
            {
                sp_state = SPCGC;
                break;
            }
            if (c[m] == ')')
            {
                sp_state = SPCR1;
                break;
            }
            if (c[m] == ' ')
            {
                sp_state = SPCR2;
                break;
            }
            neg = false;
            gsp(ns_ng);
            sp_state = SPCPRC;
            break;
        case SPCR1:
            if (tail == ')')
            {
                sp_state = SPCR3;
                break;
            }
            sp_state = OSH208;
            break;
        case SPCR2:
            if (tail != ')')
            {
                sp_state = SPCR3;
                break;
            }
            sp_state = OSH206;
            break;
        case SPCR3:
            gsp(ns_ngw);
            return true;
        case SPCESC:
            if (!get_csmb(&code, id, &lid))
            {
                sp_state = OSH200;
                break;
            }
            gsp(ns_sc);
            if (left_part)
                gsymbol(&code);
            sp_state = SPCBLO;
            break;
        case SPCSP:
            EH_ROMA0;
            if (!get_id(id, &lid))
            {
                sp_state = OSH203;
                break;
            }
            if (strncmp(stmlbl, id, lid) == 0 && (lid == MAX_ID_LEN || stmlbl[lid] == ' '))
                pchosh("209 specifier is defined through itself");
            T_U *p = spref(id, lid, tail);
            gsp(ns_cll);
            if (left_part)
                j3addr(p);
            if (c[m] == ':')
            {
                sp_state = SPCGC;
                break;
            }
            sp_state = OSH204;
            break;
        case SPCA:
            EH_ROMA0;
            if (m == CUT - 1)
            {
                sp_state = OSH205;
                break;
            }
            if (c[m] != '\'')
            {
                sp_state = SPCA1;
                break;
            }
            gsp(ns_sc);
            if (left_part)
            {
                code.tag = TAGO;
                code.info.codef = NULL;
                code.info.infoc[0] = '\'';
                gsymbol(&code);
            }
            sp_state = SPCGC;
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
                code.info.infoc[0] = c[m];
                gsymbol(&code);
            }
            EH_ROMA0;
            if (m == CUT - 1)
            {
                sp_state = OSH205;
                break;
            }
            if (c[m] != '\'')
            {
                sp_state = SPCA1;
                break;
            }
            EH_ROMA0;
            if (c[m] == '\'')
            {
                sp_state = SPCA1;
                break;
            }
            sp_state = SPCBLO;
            break;
        case SPCES:
            gsp(ns_s);
            sp_state = SPCGC;
            break;
        case SPCEB:
            gsp(ns_b);
            sp_state = SPCGC;
            break;
        case SPCEW:
            gsp(ns_w);
            sp_state = SPCGC;
            break;
        case SPCEF:
            gsp(ns_f);
            sp_state = SPCGC;
            break;
        case SPCEN:
            gsp(ns_n);
            sp_state = SPCGC;
            break;
        case SPCER:
            gsp(ns_r);
            sp_state = SPCGC;
            break;
        case SPCEO:
            gsp(ns_o);
            sp_state = SPCGC;
            break;
        case SPCEL:
            gsp(ns_l);
            sp_state = SPCGC;
            break;
        case SPCED:
            gsp(ns_d);
            sp_state = SPCGC;
            break;
        case SPCGC:
            EH_ROMA0;
            sp_state = SPCBLO;
            break;
        case OSH200:
            pchosh("200 specifier is't scaned");
            return false;
        case OSH202:
            pchosh("202 specifier has too many '(' ");
            sp_state = OSH200;
            break;
        case OSH203:
            pchosh("203 sign ':' followed by no letter or underscore within specifier ");
            sp_state = OSH200;
            break;
        case OSH204:
            pchosh("204 within specifier default last :");
            sp_state = OSH200;
            break;
        case OSH205:
            pchosh("205 within specifier default last apostroph");
            sp_state = OSH200;
            break;
        case OSH206:
            pchosh("206 default ')'in the specifier end ");
            sp_state = OSH200;
            break;
        case OSH208:
            pchosh("208 within specifier too many )");
            sp_state = OSH200;
        }
}

static void pchk(void)
{ // writing of card into sysprint
    if (!flags.uzhe_krt && sysprint != NULL)
    {
        flags.uzhe_krt = true;
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
            fputs(tmpstr, sysprint);
        }
    }
    return;
}

static void pchk_t(void)
{ // card writing into systerm
    if (!flags.uzhekrt_t)
    {
        flags.uzhekrt_t = true;
        card[CUT] = '\0';
        if (!_eoj)
        {
            char tmpstr[CUT + 28];
            sprintf(tmpstr, "%4d %s\n", cdnumb, card);
            fputs(tmpstr, systerm);
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
        char id[MAX_ID_LEN];
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
        char id[MAX_ID_LEN];
        size_t lid;
        if (!get_id(id, &lid))
            break;
        char ide[MAX_EXT_ID_LEN];
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
            lide = lid > MAX_EXT_ID_LEN ? MAX_EXT_ID_LEN : lid;
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
    char id[MAX_ID_LEN];
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
    pchosh("130 invalid record format");
    return;
}

static bool get_csmb(T_LINKTI *code, char id[MAX_ID_LEN], size_t *lid) // procedure read multiple symbol
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
            pchosh(osh111);
            break;
        }
        EH_ROMA0;
        if (!get_id(id, lid))
        {
            pchosh("112 unknown type of the multiple symbol");
            return false;
        }
        code->info.codef = (uint8_t *)fnref(id, *lid);
        code->tag = TAGF;
    } while (false);
    return true;
}

static bool get_id(char id[MAX_ID_LEN], size_t *lid)
{ // read identifier
    memset(id, ' ', MAX_ID_LEN);
    if (class[m] != 'L' && c[m] != '_')
        return false;
    id[0] = (char)toupper(c[m]);
    for (*lid = 1; *lid < MAX_ID_LEN; (*lid)++)
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
        sprintf(osh113, "113 identifier length > %d", MAX_ID_LEN);
        pchosh(osh113);
    }
    return true;
}

// read external identifier
static bool get_idm(char id[MAX_EXT_ID_LEN], size_t *lid)
{
    if (class[m] != 'L' && c[m] != '_')
        return false;
    id[0] = (char)toupper(c[m]);
    for (*lid = 1; *lid < MAX_EXT_ID_LEN; (*lid)++)
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
        sprintf(osh114, "114 external identifier length > %d", MAX_EXT_ID_LEN);
        pchosh(osh114);
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
    if (options.source)
        fputs(pr_line, sysprint);
    fputs(pr_line, systerm);
    cdnumb = 0;
    if (kolosh != 0)
        sprintf(pr_line,
                "errors   = %-3d         obj_length(bytes) = %zu\n", kolosh, mod_length);
    else
        sprintf(pr_line,
                "                       obj_length(bytes) = %zu\n", mod_length);
    if (options.source)
        fputs(pr_line, sysprint);
    fputs(pr_line, systerm);
    GET_time();
    return;
}

void oshibka(void)
{
    pchk();
    pchk_t();
    kolosh++;
    return;
}

//----------  end of file REFAL.C  -------------------
