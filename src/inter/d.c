/*--------- file -- DEBUG.C ----------------*/
/*       REFAL-Debugger functions           */
/*     Last edition date : 19.06.2024       */
/*------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "refal.def"
#include "d.h"
#include "rfintf.h"
#include "rfrun1.h"

typedef struct DET_TAB
{
    struct DET_TAB *det_next;
    char gt;
    char ge;
    char eq;
    char ne;
    char lt;
    char le;
    char tr;
    char *det_id;
} DET_TAB;
static DET_TAB *last_det = NULL;
static DET_TAB *det_table;
static DET_TAB dummy_det_table = {NULL, 0, 0, 0, 0, 0, 0, 0, NULL};

static unsigned int trace_cond = FALSE;
static unsigned int trap_cond = FALSE;
static unsigned int ge_all = TRUE;
static unsigned int eq_all = TRUE;
static unsigned short int e1empty = FALSE;
static unsigned int was_ge;
static unsigned int was_le;
static unsigned int was_eq;
static unsigned long int s_from = 0L;
static unsigned long int s_upto = 0L;
static unsigned long int s_stop = 2147483647L;
static unsigned int nogcl = 0; /* garbage collection counter */
static unsigned int s_arg;
static unsigned int l_arg;
static char buff_id[100];
static char buff[100];
static char *arg = buff;
static unsigned long int printed_step;
static unsigned long int curr_step;
static unsigned long int euc_step;
static unsigned long int res_step;
static unsigned long int curr_step1;
static unsigned long int curr_step2;
static const linkcb *nextk;
static const linkcb *res_prevk;
static const linkcb *res_nextd;
static linkcb *pk;
static linkcb *prevk;
static linkcb *nextd;
static linkcb *dot1;
static const linkcb *prevk1, *nextd1;
static linkcb *dot2;
static const linkcb *prevk2, *nextd2;

static void init_det_flags();
static void get_arg();
static unsigned int get_det();
static unsigned int get_numb(const unsigned long int *numb);
static unsigned int get_yn(const char *b);
static void dbapp(st *ss_st);
static void getpf(const st *ss_st);
static void one_step(st *ss_st);
static void pr_euc();
static void pr_finres(unsigned long int xstep, const linkcb *xprevk, const linkcb *xnextd);
static void pr_imres();
static void pr_step();

void rfdbg(st *s_st)
{
    /* read task for debugging */
    init_det_flags();
    /*----------------------------------*/
    printf("\n ***** REFAL debugger ***** \n");

    printf("\n > (function list) : ");
    fgets(buff, 100, stdin);
    unsigned int i;
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        ge_all = FALSE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->gt = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n >= (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        ge_all = FALSE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->ge = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n = (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        eq_all = FALSE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->eq = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n != (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->ne = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n < (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->lt = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n <= (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = TRUE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->le = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n TRAP (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trap_cond = TRUE;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->tr = 1;
            arg = arg + l_arg + s_arg;
        }
    }
R1:
    printf("\n STOP (step number) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
        if (get_numb(&s_stop) == 0)
            goto R1;
R2:
    printf("\n FROM (step number) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
        if (get_numb(&s_from) == 0)
            goto R2;
R3:
    printf("\n TO (step number) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
        if (get_numb(&s_upto) == 0)
            goto R3;
R4:
    printf("\n E1= (y/n) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
        if (get_yn(buff + i) == 0)
            goto R4;
    /*  set FROM and TO  */
    if (!s_from && (s_upto || trace_cond))
        s_from = 1;
    if (!s_upto && s_from)
        s_upto = 0x7FFFFFFFL;
    /*==================================*/
    /*  initialization  */
    dba = dbapp;
    printed_step = 0;
    euc_step = 0;
    res_step = 0;
    res_prevk = res_nextd = NULL;
    /* station "not yet" */
NOT_YET:
    if (s_st->dot == NULL)
        goto DONE;
    if (s_stop < s_st->step)
        goto ABEND;
    getpf(s_st);
    if (!ge_all && !(det_table->ge) && !(det_table->gt))
    {
        if (det_table->tr)
            goto TRAP;
        one_step(s_st);
        if (s_st->state != 1)
            goto ABEND;
        goto NOT_YET;
    }
    /* enter into station "is already"  */
    if ((!ge_all && !(det_table->ge)) || det_table->gt)
        was_ge = FALSE;
    else
    {
        was_ge = TRUE;
        if (!ge_all)
            pr_euc();
    }
    /*  cut   */
    curr_step1 = curr_step;
    prevk1 = prevk;
    nextd1 = nextd;
    if (pk->info.codep == NULL)
        dot1 = NULL;
    else
    {
        dot1 = pk->info.codep;
        pk->info.codep = NULL;
    }
    /* "is already" station  */
ALREADY:
    if ((det_table->le) || (det_table->lt))
    {
        /*  "isn't already" */
        if (det_table->lt)
            was_le = FALSE;
        else
        {
            was_le = TRUE;
            pr_euc();
        }
        /*   cut    */
        curr_step2 = curr_step;
        prevk2 = prevk;
        nextd2 = nextd;
        if (pk->info.codep == NULL)
            dot2 = NULL;
        else
        {
            dot2 = pk->info.codep;
            pk->info.codep = NULL;
        }
        /* compute call entirely */
        while (s_st->dot != NULL)
        {
            getpf(s_st);
            if (det_table->tr)
                goto TRAP;
            one_step(s_st);
            if (s_stop < s_st->step)
                goto ABEND;
            curr_step = s_st->step + 1;
            if (s_st->state != 1)
                goto ABEND;
        }
        /*  joint   */
        curr_step = s_st->step;
        s_st->dot = dot2;
        if (was_le)
            pr_finres(curr_step2, prevk2, nextd2);
    } /* for label ALREADY */
    else
    { /* step in station "is already" */
        if (s_stop < s_st->step)
            goto ABEND;
        if ((!eq_all && !det_table->eq) || det_table->ne)
            was_eq = FALSE;
        else
        {
            was_eq = TRUE;
            pr_euc();
        }
        if (det_table->tr)
            goto TRAP;
        one_step(s_st);
        if (s_st->state != 1)
            goto ABEND;
        if (was_eq)
            pr_imres();
    }
    if (s_st->dot != NULL)
    {
        getpf(s_st);
        goto ALREADY;
    }
    /*  joint */
    s_st->dot = dot1;
    if (!ge_all && was_ge)
        pr_finres(curr_step1, prevk1, nextd1);
    goto NOT_YET;
DONE:
    printf("\nConcretization is executed ");
    goto EOJ;
TRAP:
    printf("\nFunction name trap");
    goto ABEND1;
ABEND:
    switch (s_st->state)
    {
    case 1:
        printf("\nStop on step number ");
        break;
    case 2:
        printf("\nRecognition impossible");
        break;
    case 3:
        printf("\nFree memory exhausted ");
    }
    getpf(s_st);
ABEND1:
    printf("\nLeading functional term: ");
    rfpexm("     ", prevk, nextd);
EOJ:
    printf("\nCompleted steps number = %ld", s_st->step);
    printf("\nView field: ");
    rfpexm("     ", s_st->view, s_st->view);
    if ((s_st->store)->next != s_st->store)
    {
        printf("\nBurried: ");
        rfpexm("     ", s_st->store, s_st->store);
    }
    if (nogcl != 0)
        printf("\nGarbage collection number = %d", nogcl);
    rfcanc(s_st);
    rftermm();

    /* BLF */
    printf("\n");

    exit(0);
}

static void dbapp(st *ss_st)
{
    const linkcb *v1 = prevk;
    const linkcb *v2 = nextd;
    linkcb *v3 = pk;
    const linkcb *v4 = nextk;
    unsigned long int v5 = res_step;
    const linkcb *v6 = res_prevk;
    const linkcb *v7 = res_nextd;
NOT_YET:
    if (ss_st->dot == NULL)
        goto DO;
    if (s_stop < ss_st->step)
        goto ABEND;
    getpf(ss_st);
    if (!ge_all && !(det_table->ge) && !(det_table->gt))
    {
        if (det_table->tr)
            goto TRAP;
        one_step(ss_st);
        if (ss_st->state != 1)
            goto AB;
        goto NOT_YET;
    }
    /* enter into station "is already"  */
    if ((!ge_all && !(det_table->ge)) || det_table->gt)
        was_ge = FALSE;
    else
    {
        was_ge = TRUE;
        if (!ge_all)
            pr_euc();
    }
    /*  cut   */
    curr_step1 = curr_step;
    prevk1 = prevk;
    nextd1 = nextd;
    if (pk->info.codep == NULL)
        dot1 = NULL;
    else
    {
        dot1 = pk->info.codep;
        pk->info.codep = NULL;
    }
    /* "is already" station  */
ALREADY:
    if ((det_table->le) || (det_table->lt))
    {
        /*  "isn't already" */
        if (det_table->lt)
            was_le = FALSE;
        else
        {
            was_le = TRUE;
            pr_euc();
        }
        /*   cut    */
        curr_step2 = curr_step;
        prevk2 = prevk;
        nextd2 = nextd;
        if (pk->info.codep == NULL)
            dot2 = NULL;
        else
        {
            dot2 = pk->info.codep;
            pk->info.codep = NULL;
        }
        /* compute call entirely */
        while (ss_st->dot != NULL)
        {
            getpf(ss_st);
            if (det_table->tr)
                goto TRAP;
            one_step(ss_st);
            if (s_stop < ss_st->step)
                goto ABEND;
            curr_step = ss_st->step + 1;
            if (ss_st->state != 1)
                goto AB;
        }
        /*  joint   */
        curr_step = ss_st->step;
        ss_st->dot = dot2;
        if (was_le)
            pr_finres(curr_step2, prevk2, nextd2);
    } /* for label ALREADY */
    else
    { /* step in station "is already" */
        if (s_stop < ss_st->step)
            goto ABEND;
        if ((!eq_all && !det_table->eq) || det_table->ne)
            was_eq = FALSE;
        else
        {
            was_eq = TRUE;
            pr_euc();
        }
        if (det_table->tr)
            goto TRAP;
        one_step(ss_st);
        if (ss_st->state != 1)
            goto AB;
        if (was_eq)
            pr_imres();
    }
    if (ss_st->dot != NULL)
    {
        getpf(ss_st);
        goto ALREADY;
    }
    /*  joint */
    ss_st->dot = dot1;
    if (!ge_all && was_ge)
        pr_finres(curr_step1, prevk1, nextd1);
    goto NOT_YET;
TRAP:
    printf("\nFunction name trap");
ABEND:
ABEND1:
    printf("\nLeading functional term: ");
    rfpexm("     ", prevk, nextd);
EOJ:
    printf("\nCompleted steps number = %ld", ss_st->step);
    printf("\nView field: ");
    rfpexm("     ", ss_st->view, ss_st->view);
    if (ss_st->store->next != ss_st->store)
    {
        printf("\nBurried: ");
        rfpexm("     ", ss_st->store, ss_st->store);
    }
    if (nogcl != 0)
        printf("\nGarbage collection number = %d", nogcl);
    exit(0);
LACK:
    printf("\nRefal debugger: no memory for initialization");
    rftermm();
    exit(0);
DO:
AB:
AB1:
    prevk = v1;
    nextd = v2;
    pk = v3;
    nextk = v4;
    res_step = v5;
    res_prevk = v6;
    res_nextd = v7;
    return;
}

/*    procedures    */
static void init_det_flags()
{
    for (DET_TAB *det = last_det; det != NULL; det = det->det_next)
    {
        free(det->det_id);
        DET_TAB *det1 = det;
        free(det1);
    }
    last_det = NULL;
}

static void one_step(st *ss_st)
{
    ss_st->stop = ss_st->step + 1;
AGAIN:
    rfrun(ss_st);
    if (ss_st->state != 3)
        goto RET;
    if (refal.dvar != NULL)
        nogcl++;
    if (lincrm())
        goto AGAIN;
    else
        goto RET;
RET:
    if (e1empty && (ss_st->state == 2))
    {
        pr_step();
        if (euc_step != curr_step)
        {
            euc_step = curr_step;
            rfpexm("       leading term : ", prevk, nextd);
        }
        printf("\n*** recognition impossible ");
        printf("\n*** change leading term by empty term and continue ***");
        ss_st->dot = pk->info.codep;
        rfdel(prevk, nextd);
        ss_st->state = 1;
        ss_st->step++;
    };
    return;
}

static void pr_step()
{
    /*printf("\nprstep: curr=%ld printed=%ld",curr_step,printed_step);*/
    if (curr_step != printed_step)
    {
        printf("\n***** step %ld", curr_step);
        printed_step = curr_step;
    }
    return;
}

static void pr_euc()
{
    if ((curr_step > s_upto) || (curr_step < s_from))
        return;
    if (euc_step != curr_step)
    {
        euc_step = curr_step;
        if ((res_step != curr_step - 1) || (res_prevk != prevk) ||
            (res_nextd != nextd))
        {
            pr_step();
            rfpexm("      leading term : ", prevk, nextd);
        }
    }
    return;
}

static void pr_imres()
{
    if ((curr_step > s_upto) || (curr_step < s_from))
        return;
    pr_step();
    rfpexm("      result : ", prevk, nextd);
    res_step = curr_step;
    res_prevk = prevk;
    res_nextd = nextd;
    return;
}

static void pr_finres(unsigned long int xstep, const linkcb *xprevk, const linkcb *xnextd)
{
    if ((curr_step > s_upto) || (curr_step < s_from))
        return;
    pr_step();
    if ((curr_step == res_step) && (res_prevk == xprevk) &&
        (res_nextd == xnextd))
    {
        if (xstep == curr_step)
            return;
        printf("\n----- this is result of call on step %ld", xstep);
    }
    else
    {
        if (xstep == curr_step)
        {
            pr_imres();
            return;
        }
        printf("\n----- result of call on step %ld : ", xstep);
        rfpexm("     ", xprevk, xnextd);
        res_step = curr_step;
        res_prevk = xprevk;
        res_nextd = xnextd;
    }
    return;
}

static void getpf(const st *ss_st)
{
    curr_step = ss_st->step + 1;
    pk = ss_st->dot->info.codep;
    prevk = pk->prev;
    nextd = ss_st->dot->next;
    nextk = pk->next;
    unsigned int i;
    unsigned char id_l;
    const unsigned char *p_id;
    if (nextk->tag != TAGF)
    {
        buff_id[0] = '%';
        buff_id[1] = '\0';
    }
    else
    {
        p_id = nextk->info.codef - 1;
        id_l = *p_id;
        p_id -= id_l;
        for (i = 0; i < id_l; i++)
            buff_id[i] = rfcnv(*(p_id + i)); /* kras */
        buff_id[id_l] = '\0';
    }
    det_table = last_det;
SEARCH:
    if (strlen(buff_id) == 0)
        det_table = NULL;
    else
        while (det_table != NULL)
        {
            for (i = 0; buff_id[i] != '\0'; i++)
                if (*(det_table->det_id + i) != buff_id[i])
                    break;
            if (buff_id[i] == '\0' && *(det_table->det_id + i) == '\0')
                break;
            det_table = det_table->det_next;
        }
    if (det_table == NULL)
        det_table = &dummy_det_table;
    return;
}

static void get_arg()
{
    for (l_arg = 0;; l_arg++)
        if ((*(arg + l_arg) = rfcnv(*(arg + l_arg))) == '\n' || *(arg + l_arg) == ' ' || *(arg + l_arg) == '\0' || *(arg + l_arg) == ',')
            break;
    for (s_arg = 0; (*(arg + s_arg) == ' ') || (*(arg + s_arg) == ','); s_arg++)
        ;
    return;
}

static unsigned int get_det()
{
    det_table = last_det;
    while (det_table != NULL)
    {
        if (strncmp(det_table->det_id, arg, l_arg) == 0)
        {
            if (*(det_table->det_id + l_arg) == '\0')
                return TRUE;
        }
        det_table = det_table->det_next;
    }
    if ((det_table = (DET_TAB *)malloc(sizeof(DET_TAB))) == NULL)
    {
    AB:
        printf("\nREFAL debugger: no storage");
        exit(1);
    }
    if ((det_table->det_id = malloc(l_arg + 1)) == NULL)
        goto AB;
    for (unsigned int i = 0; i < l_arg; i++)
        *(det_table->det_id + i) = *(arg + i);
    *(det_table->det_id + l_arg) = '\0';
    det_table->det_next = last_det;
    last_det = det_table;
    det_table->ge = 0;
    det_table->gt = 0;
    det_table->eq = 0;
    det_table->ne = 0;
    det_table->le = 0;
    det_table->lt = 0;
    det_table->tr = 0;
    return TRUE;
}

static unsigned int get_numb(const unsigned long int *numb)
{
    if (sscanf(buff, "%ld", numb) == 0 || *numb < 1L)
    {
        printf("\n                        Invalid number; repeat please.");
        return FALSE;
    }
    return TRUE;
}

static unsigned int get_yn(const char *b)
{
    if (*b != 'y' && *b != 'n')
    {
        printf("\n                        Answer is \"y/n\"; repeat please.");
        return FALSE;
    }
    if (*b == 'y')
        e1empty = TRUE;
    return TRUE;
}
/*---------  end of file DEBUG.C -----------*/
