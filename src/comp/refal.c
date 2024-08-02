//-----------  file  --  REFAL.C -------------
//      The main file of refal compiler
//       Last modification : 11.07.24
//--------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "refal.def"
#include "refal.h"
#include "cerr.h"
#include "cj.h"
#include "cgop.h"
#include "plc.h"
#include "cs.h"
#include "ccst.h"

#define EH                               \
    if (m != 71)                         \
    {                                    \
        m++;                             \
        if ((m == 71) && (c[71] != ' ')) \
        {                                \
            rdcard();                    \
            if (_eoj)
#define ROMA \
    return;  \
    }        \
    }
#define ROMA0 \
    return 0; \
    }         \
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
    SCNVI,
    SCNKK,
    SCNK,
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
    unsigned int was_72 : 1;
    unsigned int uzhe_krt : 1;
    unsigned int was_err : 1;
    unsigned int uzhe_zgl : 1;
    unsigned int uzhekrt_t : 1;
} flags;

FILE *sysprint, *systerm;
FILE *syslin; // for assem
FILE *systxt; // for module names

uint16_t nommod;
char parm_i[40]; // sourse file name
// Aleksandr Bocharov   // compiler version
const char vers_i[] = "refal2_new  version 0.0.1-20240615 (c) Aleksandr Bocharov (c) Refal-2 Team";
char mod_i[13]; // 8+4+1 (xxxxxxxx.yyy0)

static FILE *sysin;
static size_t m; // current symbol number
static char strg_c[78];
static size_t lbl_leng;
static bool empcard;  // flags for empty card
static char card[81]; // card buffer (input)
static const char *card72 = card;
static unsigned int cdnumb; // card number   // kras
static bool dir;            // L,R - flag
static unsigned int kolosh;
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
static char *c = strg_c + 6;
static char class72[78];
static char *class = class72 + 6;
static bool scn_station; // scanner station - in(1),out(0) literal chain
static bool left_part;
static char *sarr[7]; // abbreviated specifier table
static char stmlbl[40];
static char prevlb[40];
static char stmkey[6];
static size_t fixm;       // start sentence position
static char mod_name[9];  // module name                 // kras
static size_t mod_length; // module length   // kras
static bool again;        // next module processing feature
static bool _eoj;         // "sysin" end flag           // kras

static void lblkey(unsigned int pr);
static void pch130();
static void blout();
static void trprev();
static void ilm(void (*prog)(const char *, unsigned int, const char *, unsigned int));
static void il(void (*prog)(const char *, unsigned int));
static void equ();
static void pchzkl();
static void pchk();
static void gsp(char n);
static bool specif(char tail);
static bool get_id(char id[40], unsigned int *lid);
static bool get_idm(char id[8], unsigned int *lid);
static bool get_csmb(T_LINKTI *code, char id[40], unsigned int *lid);

typedef struct timespec timespec;
static timespec t0;

static void SET_time()
{
    timespec_get(&t0, TIME_UTC);
    return;
}

static void GET_time()
{
    timespec t1;
    timespec_get(&t1, TIME_UTC);
    int32_t in = t1.tv_nsec - t0.tv_nsec;
    uint32_t is = difftime(t1.tv_sec, t0.tv_sec);
    if (in < 0)
    {
        in += 1000000000;
        is--;
    }
    const uint32_t im = is / 60;
    is %= 60;
    char s[25];
    sprintf(s, "%02ld:%02ld.%09ld", im, is, in);
    printf("                       elapsed time      = %s\n", s);
    return;
}

int main(int argc, char *argv[])
{
    systerm = NULL;

    /*   qindex = index_x ("abcd","xbc");

       printf ("qindex =%d\n", qindex);

       cmpstr(3,"abcd","xbc");
    */

    nommod = 0;
    printf("\n"); // BLF
    printf("%s", vers_i);
    if (argc < 2)
    {
        // BLF      printf("\nSer. No %s",regnom);
        printf("\n"); // BLF
        printf("\nSyntax: refal source_file [(option,...,option)]");
        printf("\nOptions:");
        printf("\n   mm  multi_module");
        printf("\n   nn  no_function_names");
        printf("\n   ns  no_source_listing");

        /* BLF,   printf("\n   as  assembler_module");
           assembler generated always
        */
        printf("\n   fn  full_names");
        printf("\n   cm  minimal_memory_for_compiler");
        printf("\n\n"); // BLF
        exit(1);
    };

    char parm[40];
    size_t i;
    for (i = 0; (parm[i] = *(argv[1] + i)) != '\0'; i++)
        ;

    // BLF  if ( index(parm,strlen(parm),".",1) < 0 ) strcat(parm,".ref");
    if (index_x(parm, ".") < 0)
        strcat(parm, ".ref");

    printf("\n%s:\n", parm);
    for (i = 0; i <= (strlen(parm) + 1); i++)
        parm_i[i] = parm[i];
    sysin = fopen(parm, "r");
    if (sysin == NULL)
    {
        printf("Can't open %s\n", parm);
        exit(1);
    };
    systerm = stdout;
    sysprint = NULL;

    SET_time();

    options.source = 1;
    options.stmnmb = 0;
    options.extname = 0;
    options.multmod = 0;
    options.names = 1;
    options.mincomp = 0;
    for (size_t j = 2; j < argc; ++j)
    {
        for (i = 0; (parm[i] = *(argv[j] + i)) != '\0'; i++)
            ;
        if (parm[0] == '(')
        {
            for (int32_t i = 1; (i < 40) && (parm[i] != ')') && (parm[i] != '\0');)
            {
                int32_t temp;
                if (strncmp((parm + i), "nn", 2) == 0) //  kras
                    options.names = 0;
                else if (strncmp((parm + i), "ns", 2) == 0)
                    options.source = 0;
                else if (strncmp((parm + i), "fn", 2) == 0)
                    options.extname = 1;
                else if (strncmp((parm + i), "cm", 2) == 0)
                    options.mincomp = 1;
                else if (*(parm + i) == 'm')
                    options.multmod = 1;
                else
                {
                    for (temp = 0; *(parm + temp) != '\0'; temp++)
                        ;
                    temp--;
                    if (*(parm + temp) == ')')
                        *(parm + temp) = '\0';
                    printf("Unknown option: %s\n", (parm + i));
                    printf("Options may be: ns,nn,mm,fn,cm\n");
                    exit(1);
                }
                temp = i;
                // BLF  i = index((parm + i),40-i,",",1) + 1 ;
                i = index_x((parm + i), ",") + 1;
                if (i == 0)
                {
                    // BLF     i = index((parm + temp),40-temp,")",1) ;
                    i = index_x((parm + temp), ")");
                    if (i == -1)
                    {
                        printf("Missing ')' in option definition\n");
                        exit(1);
                    }
                }
                i += temp;
            } // end for
            for (i = 0; (parm[i] = *(argv[1] + i)) != '\0'; ++i)
                ;
        } // end if
        else
        {
            printf("Illegal options definition: %s\n", parm);
            exit(1);
        }
    } // end for
    for (i = 0; ((parm[i] = *(argv[1] + i)) != '\0') && (parm[i] != '.'); ++i)
        ;
    parm[i] = '\0';
    if (options.source == 1)
    {
        strcat(parm, ".lst");
        if ((sysprint = fopen(parm, "w")) == NULL)
        {
            printf("Can't open %s\n", parm);
            exit(8);
        }
    }
    for (i = 0; ((parm[i] = *(argv[1] + i)) != '\0') && (parm[i] != '.'); ++i)
        ;
    parm[i] = '\0';
    if (options.multmod == 1)
    {
        strcat(parm, ".txt");
        systxt = fopen(parm, "w");
        if (systxt == NULL)
        {
            printf("Can't open %s\n", parm);
            exit(8);
        }
    }
    else
    {
        strcat(parm, ".asm");
        syslin = fopen(parm, "w");
        if (syslin == NULL)
        {
            printf("Can't open %s\n", parm);
            exit(8);
        }
    }
    //  print of page title missing here
    flags.was_err = 0;
    flags.uzhe_zgl = 0;
    cdnumb = 0;
    scn_.nomkar = 0;
    T_MOD_STATES mod_state = START_OF_MODULE;
    while (true)
        switch (mod_state)
        {
        case START_OF_MODULE:
            //  time processing missing here
            kolosh = 0;
            nommod++;
            flags.was_72 = 0;
            scn_.curr_stmnmb = 0;
            _eoj = false;
            card[80] = '\n';
            prevlb[0] = '\0';
            mod_length = 0; // kras
            for (i = 0; i < 9; i++)
                mod_name[i] = '\0'; // kras
            for (i = 0; i < 7; ++i)
                sarr[i] = NULL;
            // "start" - directive work
            lblkey(0);
            if (_eoj)
            {
                mod_state = END_OF_SYSIN;
                break;
            }
            s_init();
            if ((strncmp(stmkey, "start", 5) != 0) && (strncmp(stmkey, "START", 5) != 0) && (strncmp(stmkey, "CTAPT", 5) != 0))
            {
                pchosh("001 START-directive missing");
                scn_.modnmlen = 0;
                jstart(mod_name, 0);
                mod_state = KEYS;
                break;
            }
            strncpy(mod_name, stmlbl, 8 > lbl_leng ? lbl_leng : 8);
            for (i = 0; i < (8 > lbl_leng ? lbl_leng : 8); i++)
                mod_i[i] = mod_name[i];
            mod_i[i] = 0;
            strncpy(scn_.modname_var, stmlbl, lbl_leng);
            scn_.modnmlen = lbl_leng;
            jstart(mod_name, 8 < lbl_leng ? 8 : lbl_leng);
            mod_state = NEXT_STM;
            break;
        case NEXT_STM:
            // read of next sentence
            lblkey(0);
            mod_state = KEYS;
            break;
        case KEYS:
            if ((strncmp(stmkey, "l ", 2) == 0) || (strncmp(stmkey, "L ", 2) == 0))
            {
                dir = true;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            else if ((strncmp(stmkey, "r ", 2) == 0) || (strncmp(stmkey, "R ", 2) == 0))
            {
                dir = false;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            else if ((strncmp(stmkey, "start", 5) == 0) || (strncmp(stmkey, "START", 5) == 0) || (strncmp(stmkey, "CTAPT", 5) == 0))
                pchosh("002 too many start-directive");
            else if ((strncmp(stmkey, "end", 3) == 0) || (strncmp(stmkey, "END", 3) == 0) || (strncmp(stmkey, "��H��", 5) == 0))
            {
                if (prevlb[0] != '\0')
                    sempty(prevlb, strlen(prevlb));
                again = true;
                mod_state = END_STATEMENT;
                break;
            }
            else if ((strncmp(stmkey, "entry", 5) == 0) || (strncmp(stmkey, "ENTRY", 5) == 0) || (strncmp(stmkey, "BXO�H", 5) == 0))
                ilm(sentry);
            else if ((strncmp(stmkey, "extrn", 5) == 0) || (strncmp(stmkey, "EXTRN", 5) == 0) || (strncmp(stmkey, "BHE�H", 5) == 0))
                ilm(sextrn);
            else if ((strncmp(stmkey, "empty", 5) == 0) || (strncmp(stmkey, "EMPTY", 5) == 0) || (strncmp(stmkey, "��CTO", 5) == 0))
                il(sempty);
            else if ((strncmp(stmkey, "swap", 4) == 0) || (strncmp(stmkey, "SWAP", 4) == 0) || (strncmp(stmkey, "OBMEH", 5) == 0))
                il(sswap);
            else if ((strncmp(stmkey, "swop", 4) == 0) || (strncmp(stmkey, "SWOP", 4) == 0))
                il(sswap);
            else if ((strncmp(stmkey, "s ", 2) == 0) || (strncmp(stmkey, "S ", 2) == 0))
            {
                trprev();
                spdef(stmlbl, lbl_leng);
                specif(' ');
            }
            else if ((strncmp(stmkey, "equ", 3) == 0) || (strncmp(stmkey, "EQU", 3) == 0) || (strncmp(stmkey, "�KB", 3) == 0))
                equ();
            else if (stmkey[0] == ' ')
            {
                trprev();
                if (lbl_leng != 0)
                {
                    for (i = 0; i < lbl_leng; i++)
                        prevlb[i] = stmlbl[i];
                    prevlb[i] = '\0';
                }
            }
            else
            {
                m = fixm; // return to left
                dir = true;
                trprev();
                cst(dir, stmlbl, lbl_leng);
            }
            if (!_eoj)
                mod_state = NEXT_STM;
            else
                mod_state = END_IS_MISSING;
            break;
        case END_IS_MISSING:
            pchosh("003 end directive missing");
            kolosh++;
            again = false;
            mod_state = END_STATEMENT;
            break;
        case END_STATEMENT:
            s_end();
            if (kolosh != 0)
            {
                flags.was_err = 1;
                mod_length = 0;
            }
            else
            {
                jend();
                mod_length = jwhere();
            }
            s_term();
            pchzkl();
            if (_eoj || options.multmod == 0)
                mod_state = END_OF_SYSIN;
            else if (again)
                mod_state = START_OF_MODULE;
            else
                mod_state = END_OF_SYSIN;
            break;
        case END_OF_SYSIN:
            fclose(sysin);
            if (options.source == 1)
                fclose(sysprint);
            if (options.multmod == 0)
            {
                mod_length = jwhere();
                fclose(syslin);
                if ((mod_length == 0) || (flags.was_err != 0))
                    unlink(parm);
            }
            if (flags.was_err != 0)
            {
                if (options.multmod == 1)
                    unlink(parm);
                exit(1);
            }
            else
            {
                if (nommod <= 1 && options.multmod == 1)
                    unlink(parm); // for multimod.
                exit(0);
            }
            return 0;
        }
} // main program  end

static void trprev()
{ // perenos poslednej pustoj metki w tekuschuju
    unsigned int n = strlen(prevlb);
    if ((n != 0) && (lbl_leng == 0))
    {
        strcpy(stmlbl, prevlb);
        lbl_leng = n;
    }
    else if (n != 0)
        sempty(prevlb, n);
    prevlb[0] = '\0';
    return;
}

static void rdline(char *s)
{ // read 80 symbols from sysin
    empcard = true;
    size_t i;
    int c;
    for (i = 0; ((c = getc(sysin)) != '\n') && (c != EOF) && (i < 80); i++)
    {
        if (c == '\t')
        {
            const size_t k = 8 - (i & 7);
            for (size_t j = 0; j < k; j++)
                *(s + i + j) = ' ';
            i += (k - 1);
        }
        else if ((c < ' ') && (c > 0))
            *(s + i) = ' ';
        else
        {
            *(s + i) = c;
            empcard = false;
        }
    }
    if ((c == EOF) && (i == 0))
        _eoj = true;
    for (; i < 80; i++)
        *(s + i) = ' ';
    return;
}

static void translate(const char *str, char *class1)
{ // L,D,* - classification procedure
    for (size_t i = 0; i < 72; ++i)
    {
        *(class1 + i) = '*';
        const int j = (int)(*(str + i));
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
        if (((j > -129) && (j < -80)) || ((j > -33) && (j < -16)))
            *(class1 + i) = 'L';
        if ((j == 35) || (j == 95))
            *(class1 + i) = 'L';
    }
    return;
}

static bool komm()
{
    const char *k;
    for (k = c; (*k == ' ') || (*k == '\t'); k++)
        ;
    if (*k == '*')
        return true;
    else
        return false;
}

static void rdcard()
{ // read card procedure
    while (true)
    {
        rdline(card);
        strncpy(c, card72, 72);
        translate(card72, class);
        ++scn_.nomkar;
        ++cdnumb;
        //  printf("\ncard %d",cdnumb);
        flags.uzhe_krt = 0;
        flags.uzhekrt_t = 0;
        if (options.source == 1)
            pchk();
        if ((flags.was_72 == 0) && komm())
            continue;
        if (empcard)
        {
            if (_eoj)
                return;
            continue;
        }
        break;
    }
    if (*(c + 71) != ' ')
        flags.was_72 = 1;
    else
        flags.was_72 = 0;
    if (*(c + 71) != ' ')
        *(c + 71) = '+'; //!!!
    m = 0;
    return;
}

//    directive label and keyword extraction
static void lblkey(unsigned int pr)
{
    if (pr == 0)
        while (true)
        {
            rdcard();
            if (c[0] == ' ')
                lbl_leng = 0;
            else if (!get_id(stmlbl, &lbl_leng))
            {
                pchosh("120 the first symbol is not letter or blank");
                continue;
            }
            break;
        }
    blout();
    for (size_t i = 0; i < 6; i++)
        stmkey[i] = ' ';
    do
    {
        if (c[m] == ' ')
            break;
        fixm = m;
        uint16_t l = 0;
        while (c[m] != ' ')
        {
            if (m == 71)
            {
                const uint16_t delta = 71 - fixm;
                const int16_t fixm1 = 0 - delta;
                for (m = 0; m != delta; m++)
                {
                    c[fixm1 + m] = c[fixm + m];
                    class[fixm1 + m] = class[fixm + m];
                }
                rdcard();
                fixm = fixm1;
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

void scan()
{
    static char id[40];
    static unsigned int id_leng;
    static const uint8_t *p;
    static size_t scode;
    scn_e.code.tag = 0;
    scn_e.code.info.codef = NULL;
    scn_e.v = false;
    scn_e.spec.tag = 0;
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
            case '/':
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
                scn_state = SCNKK; // kras
                break;
            case 'k':
            case 'K':
                scn_state = SCNK;
                break;
            case '.':
            case '>':
                scn_state = SCNP; // kras
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
            if (get_csmb(&(scn_e.code), id, &id_leng) == 1)
                scn_state = EGO;
            else
                scn_state = SCNERR;
            break;
        case EGO:
            scn_e.t = 1;
            scn_state = SCNGCR;
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
            EH ROMA;
            if (c[m] == '(')
            {
                EH ROMA;
                if (left_part)
                {
                    p = scn_e.spec.info.codef = (uint8_t *)genlbl();
                    jlabel((T_U *)p);
                }
                if (specif(')'))
                {
                    EH ROMA else
                    {
                        scn_state = SCNERR;
                        break;
                    }
                }
            }
            else if (c[m] == ':')
            {
                EH ROMA;
                if (!get_id(id, &id_leng))
                {
                    scn_state = SOSH203;
                    break;
                }
                if (left_part)
                    scn_e.spec.info.codef = (uint8_t *)spref(id, id_leng, ')');
                if (c[m] == ':')
                {
                    EH ROMA else
                    {
                        scn_state = SOSH204;
                        break;
                    }
                }
            }
            scn_state = SCNVI;
            break;
        case SCNVI:
            if ((class[m] != 'L') && (class[m] != 'D'))
                scn_state = OSH102;
            else
            {
                scn_e.ci = c[m];
                scn_state = SCNGCR;
            }
            break;
        case SCNKK: // kras
            scn_e.t = 7;
            if (c[m + 1] != ' ')
            {
                c[m - 1] = '/';
                size_t i;
                for (i = 1;
                     (class[m + i] == 'L') || (class[m + i] == 'D') || (c[m + i] == '_') || (c[m + i] == '-');
                     i++)
                {
                    c[m + i - 1] = c[m + i];
                    class[m + i - 1] = class[m + i];
                }
                c[m + i - 1] = '/';
                class[m + i - 1] = '*';
                m -= 2;
            }
            scn_state = SCNGCR;
            break;
        case SCNK:
            scn_e.t = 7;
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
            EH ROMA;
            if (m == 71)
                scn_state = OSH101;
            else if (c[m] == '\'')
                scn_state = SCNCHR;
            else
            {
                scn_station = true;
                scn_state = SCNCHR;
            }
            break;
        case STATE1: // within letter chain
            if (m == 71)
                scn_state = OSH101;
            else if (c[m] != '\'')
                scn_state = SCNCHR;
            else
            {
                EH ROMA;
                if (c[m] == '\'')
                    scn_state = SCNCHR;
                else
                {
                    scn_station = false;
                    scn_state = STATE0;
                }
            }
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
                    if ((c[m + 1] >= '0') && (c[m + 1] <= '7'))
                    {
                        uint32_t j = 0;
                        for (size_t i = 1; i < 3; i++)
                            if ((c[m + i] >= '0') && (c[m + i] <= '7'))
                                j = j * 8 + c[m + i] - '0';
                            else
                            {
                                m--;
                                break;
                            }
                        m += 2;
                        c[m] = j & 255;
                    }
                    else
                        c[m] = 0;
                    break;
                default:
                    if ((c[m] >= '0') && (c[m] <= '7'))
                    {
                        uint32_t j = 0;
                        for (size_t i = 0; i < 3; i++)
                            if ((c[m + i] >= '0') && (c[m + i] <= '7'))
                                j = j * 8 + c[m + i] - '0';
                            else
                            {
                                m--;
                                break;
                            }
                        m += 2;
                        c[m] = j & 255;
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
                    *(sarr + scode) = (char *)genlbl();
                    jlabel((T_U *)(sarr + scode));
                    gsp((char)(scode + 7));
                    gsp(ns_ngw);
                };
                scn_e.spec.info.codef = (uint8_t *)*(sarr + scode);
            };
            EH ROMA;
            scn_state = SCNVI;
            break;
        case OSH101:
            pchosh("101 default of left apostroph");
            scn_state = SCNERR;
            break;
        case OSH102:
            pchosh("102 identifier index is't letter or digit");
            scn_state = SCNERR;
            break;
        case SOSH203:
            pchosh("203 sign ':' followed by no letter");
            scn_state = SCNERR;
            break;
        case SOSH204:
            pchosh("204 default last ':' within specifier");
            scn_state = SCNERR;
            break;
        case SCNGCR:
            EH ROMA;
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
    char id[255];
    unsigned int lid;
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
            case '/':
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
                sp_state = OSH206;
            else
                return true;
            break;
        case SPCL:
            if (neg)
                sp_state = OSH202;
            else
            {
                neg = true;
                gsp(ns_ng);
                sp_state = SPCGC;
            }
            break;
        case SPCR:
            if (!neg)
                sp_state = SPCR1;
            else
            {
                EH ROMA0; // kras
                blout();
                if (c[m] == '(')
                    sp_state = SPCGC;
                else if (c[m] == ')')
                    sp_state = SPCR1;
                else if (c[m] == ' ')
                    sp_state = SPCR2;
                else
                {
                    neg = false;
                    gsp(ns_ng);
                    sp_state = SPCPRC;
                }
            }
            break;
        case SPCR1:
            if (tail == ')')
                sp_state = SPCR3;
            else
                sp_state = OSH208;
            break;
        case SPCR2:
            if (tail != ')')
                sp_state = SPCR3;
            else
                sp_state = OSH206;
            break;
        case SPCR3:
            gsp(ns_ngw);
            return true;
        case SPCESC:
            if (get_csmb(&code, id, &lid) == 0)
                sp_state = OSH200;
            else
            {
                gsp(ns_sc);
                if (left_part)
                    gsymbol(&code);
                sp_state = SPCGC;
            }
            break;
        case SPCSP:
            EH ROMA0; // kras
            if (!get_id(id, &lid))
                sp_state = OSH203;
            else
            {
                if (strncmp(stmlbl, id, lid) == 0 && stmlbl[lid] == ' ')
                    pchosh("209 specifier is defined through itself");
                T_U *p = spref(id, lid, tail);
                gsp(ns_cll);
                if (left_part)
                    j3addr(p);
                if (c[m] == ':')
                    sp_state = SPCGC;
                else
                    sp_state = OSH204;
            }
            break;
        case SPCA:
            EH ROMA0; // kras
            if (m == 71)
                sp_state = OSH205;
            else if (c[m] != '\'')
                sp_state = SPCA1;
            else
            {
                gsp(ns_sc);
                if (left_part)
                {
                    code.tag = 0;
                    code.info.codef = 0L;
                    code.info.infoc[0] = '\'';
                    gsymbol(&code);
                }
                sp_state = SPCGC;
            }
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
                        if ((c[m + 1] >= '0') && (c[m + 1] <= '7'))
                        {
                            uint32_t j = 0;
                            for (size_t i = 1; i < 3; i++)
                                if ((c[m + i] >= '0') && (c[m + i] <= '7'))
                                    j = j * 8 + c[m + i] - '0';
                                else
                                {
                                    m--;
                                    break;
                                }
                            m += 2;
                            c[m] = j & 255;
                        }
                        else
                            c[m] = 0;
                        break;
                    default:
                        if ((c[m] >= '0') && (c[m] <= '7'))
                        {
                            uint32_t j = 0;
                            for (size_t i = 0; i < 3; i++)
                                if ((c[m + i] >= '0') && (c[m + i] <= '7'))
                                    j = j * 8 + c[m + i] - '0';
                                else
                                {
                                    m--;
                                    break;
                                }
                            m += 2;
                            c[m] = j & 255;
                        }
                        else
                            m--;
                    }
                code.tag = 0;
                code.info.codef = 0L;
                code.info.infoc[0] = c[m];
                gsymbol(&code);
            }
            EH ROMA0; // kras
            if (m == 71)
                sp_state = OSH205;
            else if (c[m] != '\'')
                sp_state = SPCA1;
            else
            {
                EH ROMA0; // kras
                if (c[m] == '\'')
                    sp_state = SPCA1;
                else
                    sp_state = SPCBLO;
            }
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
            EH ROMA0; // kras
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
            pchosh("203 sign ':' followed by no letter within specifier ");
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

static void pchk()
{ // writing of card into sysprint
    if (flags.uzhe_krt == 0 && sysprint != NULL)
    {
        flags.uzhe_krt = 1;
        card[72] = '\0';
        if (!_eoj)
        {
            char tmpstr[80];
            sprintf(tmpstr, "%4d %s", cdnumb, card);
            unsigned int i;
            for (i = 76; i > 4; i--)
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

static void pchk_t()
{ // card writing into systerm
    if (flags.uzhekrt_t == 0)
    {
        flags.uzhekrt_t = 1;
        card[72] = '\0';
        if (!_eoj)
        {
            char tmpstr[80];
            sprintf(tmpstr, "%4d %s\n", cdnumb, card);
            fputs(tmpstr, systerm);
        }
    }
    return;
}

static void il(void (*prog)(const char *, unsigned int)) // treatment of directives having 'EMPTY' type
{
    blout();
    while (true)
    {
        char id[40];
        unsigned int lid;
        if (!get_id(id, &lid))
            break;
        (*prog)(id, lid);
        blout();
        if (m == 71 && c[m] == ' ')
            return;
        if (c[m] == ',')
        {
            EH ROMA;
            if (c[m] == ' ')
                blout();
            continue;
        }
        break;
    }
    pch130();
    return;
}

static void ilm(void (*prog)(const char *, unsigned int, const char *, unsigned int)) // treatment of directives having 'ENTRY' type
{
    blout();
    while (true)
    {
        char id[40];
        unsigned int lid;
        if (!get_id(id, &lid))
            break;
        char ide[8];
        unsigned int lide;
        if (c[m] == '(')
        {
            EH ROMA;
            if (!get_idm(ide, &lide))
                break;
            (*prog)(id, lid, ide, lide);
            if (c[m] != ')')
                break;
            EH ROMA;
        }
        else
        {
            lide = (lid > 8) ? 8 : lid;
            strncpy(ide, id, lide);
            (*prog)(id, lid, ide, lide);
        }
        blout();
        if (m == 71 && c[m] == ' ')
            return;
        if (c[m] == ',')
        {
            EH ROMA;
            if (c[m] == ' ')
                blout();
            continue;
        }
        break;
    }
    pch130();
    return;
}

static void equ()
{ // treatement of directives having 'EQU' type
    blout();
    char id[40];
    unsigned int lid;
    do
    {
        if (!get_id(id, &lid))
            break;
        sequ(stmlbl, lbl_leng, id, lid);
        if (c[m] == ' ')
            return;
    } while (false);
    pch130();
    return;
}

static void pch130()
{
    pchosh("130 invalid record format");
    return;
}

static bool get_csmb(T_LINKTI *code, char id[40], unsigned int *lid) // procedure read multiple symbol
{
    code->tag = 0;
    code->info.codef = NULL;
    EH ROMA0; // kras
    if (class[m] == 'D')
        goto CSMBN;
    if (!get_id(id, lid))
        goto OSH112;
    code->info.codef = (uint8_t *)fnref(id, *lid);
    code->tag = TAGF;
    goto CSMBEND;
CSMBN:
    code->tag = TAGN;
    code->info.coden = 0;
    uint32_t k = c[m] - '0';
CSMBN1:
    EH ROMA0; // kras
    if (class[m] != 'D')
        goto CSMBN3;
    const uint32_t l = c[m] - '0';
    k = k * 10L + l;
    if (k <= 16777215L)
        goto CSMBN1;
CSMBN2:
    EH ROMA0; // kras
    if (class[m] == 'D')
        goto CSMBN2;
    pchosh("111 symbol-number > 16777215");
    goto CSMBEND;
CSMBN3:
    code->tag = TAGN;
    code->info.coden = k;
CSMBEND:
    if (c[m] != '/')
        goto OSH113;
    return true;
OSH112:
    pchosh("112 unknown type of the multiple symbol ");
    return false;
OSH113:
    pchosh("113 default '/' under multiple symbol ");
    return false;
}

static char convert(char cm)
{
    const int j = (int)cm;
    if ((j > 96) && (j < 123))
        cm = cm - '\40';
    if ((j > -97) && (j < -80))
        cm = cm - '\40';
    if ((j > -33) && (j < -16))
        cm = cm - 80;
    return cm;
}

static bool get_id(char id[40], unsigned int *lid)
{ // read identifier
    for (size_t i = 0; i < 40; id[i++] = ' ')
        ;
    if (class[m] != 'L')
        return false;
    id[0] = convert(c[m]);
    for (*lid = 1; *lid < 40; (*lid)++)
    {
        EH ROMA0; // kras
        if ((class[m] != 'L') && (class[m] != 'D') && (c[m] != '_') && (c[m] != '-'))
            return true;
        id[*lid] = convert(c[m]);
    }
    // if identifier length > 40 then delete tail
    while ((class[m] == 'L') || (class[m] == 'D') || (c[m] == '_') || (c[m] == '-'))
    {
        EH ROMA0;
    } // kras
    return true;
}

// read external identifier
static bool get_idm(char id[8], unsigned int *lid)
{
    if (class[m] != 'L')
        return false;
    id[0] = convert(c[m]);
    for (*lid = 1; *lid < 8; (*lid)++)
    {
        EH ROMA0; // kras
        if ((class[m] != 'L') && (class[m] != 'D'))
            return true;
        id[*lid] = convert(c[m]);
    }
    // if identifier length > 8 then delete tail
    while ((class[m] == 'L') || (class[m] == 'D'))
    {
        EH ROMA0;
    } // kras
    (*lid)++;
    return true;
}

//**********************************************************
//                  missing blanks
//       before call: (m = 71) !! (m != 71)
//  under call:((m=71)&&(c[m]=' '))!!((m!=71)&&(c[m]!=' '))
//**********************************************************
static void blout()
{
    while (true)
    {
        while ((m != 71) && (c[m] == ' '))
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

static void pchzkl()
{ // print conclusion
    char pr_line[180];
    sprintf(pr_line,
            "mod_name = %-8s    mod_length(lines) = %d\n", mod_name, cdnumb);
    if (options.source == 1)
        fputs(pr_line, sysprint);
    fputs(pr_line, systerm);
    cdnumb = 0;
    if (kolosh != 0)
        sprintf(pr_line,
                "errors   = %-3d         obj_length(bytes) = %ld\n", kolosh, mod_length);
    else
        sprintf(pr_line,
                "                       obj_length(bytes) = %ld\n", mod_length);
    if (options.source == 1)
        fputs(pr_line, sysprint);
    fputs(pr_line, systerm);
    GET_time();
    return;
}

void oshibka()
{
    pchk();
    pchk_t();
    kolosh++;
    return;
}
//----------  end of file REFAL.C  -------------------
