/*--------- file -- DEBUG.C ----------------*/
/*       REFAL-Debugger functions           */
/*     Last edition date : 19.06.2024       */
/*------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "refal.def"
#include "debug.def"
#include "rfintf.h"

extern REFAL refal;

void print_parm();
void parm_menue();

void rfdbg(st *s_st)
{
    /* read task for debugging */
    init_det_flags();
#ifdef VIDEO
    /*----------------------------------*/
    dfile = fopen("refal.cfg", "r");
    video_page = 1;
    init_WND(0);

    def_window(TITUL, 0, 0, 0, 30, 25, 0, 55, 2, B_GREEN, SWAPABLE);
    set_color(RED);
    draw_frame(0, 0, 30, 2, RED, DOUBLE);
    set_position(3, 1);
    wprintf(" REFAL Debugger (v. 1.0)");

    def_window(MESSAGE, 0, 0, 79, 2, 0, 21, 79, 23, B_BLACK, SWAPABLE);
    set_color(GREEN);
    draw_frame(0, 0, 79, 2, GREEN, DOUBLE);
    set_position(36, 0);
    wprintf(" Message ");
    set_margins(1, 1, 78, 1);
    set_position(1, 1);

    if (dfile != NULL)
        rd_deb_file();
    parm_menue();
    if (modif == '+')
        wr_deb_file();
    set_video_page(0);
    set_cursor_size(6, 7);
/*==================================*/
#else
    /*----------------------------------*/
    printf("\n ***** REFAL debugger ***** \n");

    printf("\n > (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = 1;
        ge_all = 0;
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
        trace_cond = 1;
        ge_all = 0;
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
        trace_cond = 1;
        eq_all = 0;
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
        trace_cond = 1;
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
        trace_cond = 1;
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
        trace_cond = 1;
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
        trap_cond = 1;
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
#endif
    /*  initialization  */
    dba = dbapp;
    printed_step = 0;
    euc_step = 0;
    res_step = 0;
    res_prevk = res_nextd = NULL;
    /* station "not yet" */
NOT_YET:
#ifdef VIDEO
    /*----------------------------------*/
    if (kbhit())
    {
        c2 = d_get_key();
        /*      printf("\nNot_Yet: c2=%d",c2); */
        if (c2 != 19)
        { /* ^S */
            set_video_page(1);
            set_cursor_size(32, 32);
            modif = '-';
            parm_menue();
            if (modif == '+')
                wr_deb_file();
            set_video_page(0);
            set_cursor_size(6, 7);
        }
    }
/*==================================*/
#endif
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
        was_ge = 0;
    else
    {
        was_ge = 1;
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
#ifdef VIDEO
    /*----------------------------------*/
    if (kbhit())
    {
        c2 = d_get_key(); /* get symb */
        /*printf("\nAlready: c2=%d",c2);*/
        if (c2 != 19)
        { /* ^S */
            set_video_page(1);
            set_cursor_size(32, 32);
            modif = '-';
            parm_menue();
            if (modif == '+')
                wr_deb_file();
            set_video_page(0);
            set_cursor_size(6, 7);
        }
    }
/*==================================*/
#endif
    if ((det_table->le) || (det_table->lt))
    {
        /*  "isn't already" */
        if (det_table->lt)
            was_le = 0;
        else
        {
            was_le = 1;
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
            pr_finres((long)curr_step2, prevk2, nextd2);
    } /* for label ALREADY */
    else
    { /* step in station "is already" */
        if (s_stop < s_st->step)
            goto ABEND;
        if ((!eq_all && !det_table->eq) || det_table->ne)
            was_eq = 0;
        else
        {
            was_eq = 1;
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
        pr_finres((long)curr_step1, prevk1, nextd1);
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
    int i;
    int c2;
    linkcb *v1, *v2, *v3, *v4, *v6, *v7;
    long v5;
    v1 = prevk;
    v2 = nextd;
    v3 = pk;
    v4 = nextk;
    v5 = res_step;
    v6 = res_prevk;
    v7 = res_nextd;
NOT_YET:
#ifdef VIDEO
    /*----------------------------------*/
    if (kbhit())
    {
        c2 = d_get_key();
        if (c2 != 19)
        { /* ^S */
            set_video_page(1);
            set_cursor_size(32, 32);
            modif = '-';
            parm_menue();
            if (modif == '+')
                wr_deb_file();
            set_video_page(0);
            set_cursor_size(6, 7);
        }
    }
/*==================================*/
#endif
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
        was_ge = 0;
    else
    {
        was_ge = 1;
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
#ifdef VIDEO
    /*----------------------------------*/
    if (kbhit())
    {
        c2 = d_get_key(); /* get symb */
        /*printf("\nAlready: c2=%d",c2);*/
        if (c2 != 19)
        { /* ^S */
            set_video_page(1);
            set_cursor_size(32, 32);
            modif = '-';
            parm_menue();
            if (modif == '+')
                wr_deb_file();
            set_video_page(0);
            set_cursor_size(6, 7);
        }
    }
/*==================================*/
#endif
    if ((det_table->le) || (det_table->lt))
    {
        /*  "isn't already" */
        if (det_table->lt)
            was_le = 0;
        else
        {
            was_le = 1;
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
            pr_finres((long)curr_step2, prevk2, nextd2);
    } /* for label ALREADY */
    else
    { /* step in station "is already" */
        if (s_stop < ss_st->step)
            goto ABEND;
        if ((!eq_all && !det_table->eq) || det_table->ne)
            was_eq = 0;
        else
        {
            was_eq = 1;
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
        pr_finres((long)curr_step1, prevk1, nextd1);
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
static void
init_det_flags()
{
    DET_TAB *det, *det1;
    for (det = last_det; det != NULL; det = det->det_next)
    {
        free(det->det_id);
        det1 = det;
        free(det1);
    }
    last_det = NULL;
}
static void
one_step(st *ss_st)
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

static void pr_finres(long xstep, linkcb *xprevk, linkcb *xnextd)
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

extern char rfcnv(char cm);

static void getpf(st *ss_st)
{
    int i;
    short id_l;
    char *p_id;
    curr_step = ss_st->step + 1;
    pk = ss_st->dot->info.codep;
    prevk = pk->prev;
    nextd = ss_st->dot->next;
    nextk = pk->next;
    if (nextk->tag != TAGF)
    {
        buff_id[0] = '%';
        buff_id[1] = '\0';
    }
    else
    {
        p_id = nextk->info.codef;
        p_id--;
        id_l = (short)(*p_id);
        p_id = p_id - id_l;
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
/* video-procedures */
#ifdef VIDEO
/*----------------------------------*/

static w_desc w0 = {DEF_PARM, 0, 0, 30, 14, 25, 3, 55, 17, B_BLUE,
                    ERASABLE, " Enter - select parameters ", 0, NULL};
static alt_list mf1_alt = {12, word, NULL};
static menu_field mf1 = {f_VERTICAL, (mult_list *)&mf1_alt, 4, 1, 16, 12,
                         B_BLUE, LIGHT_GREY, 12, 12, 0, 0, 3};
static elem_schem el1 = {&mf1, NULL, NULL, NULL, NULL};
static char *pmem = NULL;
static char *dwmem = NULL;

static besc()
{
    set_ret(1);
    return 0;
}

static void
    msg_print(msg) char *msg;
{
    set_video_page(1);
    set_cursor_size(32, 32);
    set_window(MESSAGE);
    set_color(RED);
    set_backgr_color(BL_BLACK);
    wput(msg, 77);
}

static void
    print_numb_parm(n) int n;
{
    if (n == 8)
        sprintf(reply_buf, "%ld", s_stop);
    if (n == 9)
        sprintf(reply_buf, "%ld", s_from);
    if (n == 10)
        sprintf(reply_buf, "%ld", s_upto);
    if (n == 11)
        if (e1empty)
            strcpy(reply_buf, "y");
        else
            strcpy(reply_buf, "n");
    if (n != 0)
        print_parm(n);
}

static void print_parm(n) int n;
{
    int i;
    DET_TAB *det_t;
    int c_color;
    int w_current = w_curr;

    set_position(9, 13);
    set_color(RED);
    wput("Esc - execute", 14);
    if (n == ALL_PARM)
    {
        for (i = 0; i < 12; i++)
        { /*!!!*/
            if (i < 8)
            {
                reply_buf[0] = '\0';
                for (det_t = last_det; det_t != NULL; det_t = det_t->det_next)
                    if (*((char *)(&(det_t->gt)) + i - 1) == 1)
                    {
                        strcat(reply_buf, det_t->det_id);
                        strcat(reply_buf, " ");
                    }
                print_parm(i);
            }
            else
                print_numb_parm(i);
        }
    }
    else
    {
        c_color = color_curr;
        set_window(PSEUDO_PARM);
        set_position(17, 1 + n);
        wprintf("%13c", ' ');
        set_position(17, 1 + n);
        set_color(CYAN);
        wput(reply_buf, 13);
        set_color(c_color);
        set_window(w_current);
    }
}

static void
    wrfabe(text) char *text;
{
    int w_current = w_curr;
    def_window(RFABEND, 0, 0, 40, 4, 20, 11, 60, 14, B_RED, SWAPABLE);
    draw_frame(0, 0, 40, 3, BLACK, DOUBLE);
    set_position(14, 0);
    wputs(" Fatal error ");
    set_position(1, 1);
    set_backgr_color(BL_RED);
    wput(text, 39);
    set_position(14, 2);
    wputs("Press any key");
    d_get_key();
    remove_window(RFABEND);
    set_window(w_current);
}

static void parm_menue()
{
    if (pmem == NULL)
    {
        if ((pmem = (char *)malloc(62 * 15)) == NULL)
        {
            wrfabe("Parm_menue: No memory");
            set_video_page(0);
            set_cursor_size(6, 7);
            exit(8);
        }
        el1.p_up = &el1;
        el1.p_down = &el1;
    }
    def_window(PSEUDO_PARM, 0, 0, 30, 14, 25, 3, 55, 17, B_BLUE, PSEUDO);
    set_window(PSEUDO_PARM);
    /* save_window ("M:", pmem);  !!! */
    user_block = rd_deb_screen;
    man_flags |= USER_EXE;
    man_flags |= PARTIAL_SUBST;
    if (dwmem == NULL)
    {
        def_w(&w0);
        set_window(PSEUDO_PARM);
        print_parm(ALL_PARM);
    }
    else
    {
        def_window(PSEUDO_PARM1, 0, 0, 30, 14, 25, 3, 55, 17, B_BLUE, PSEUDO);
        /*restore_window ("M:", dwmem); !!! */
    }
    menue(NULL, &el1, &f1);
    select_item(&mf1, mf1.curr_m_item, f_REVERSE, f_SELECT);
    /* if (dwmem == NULL) {
          dwmem = (char *)malloc (62*15);
          if (dwmem == NULL) {
             wrfabe("Parm_menue: No memory");
             set_video_page(0);
             set_cursor_size(6,7);
             exit(8);
          }
        }
        save_window ("M:", dwmem);
    set_window (PSEUDO_PARM);
    restore_window ("M:", pmem); !!! */
}

static void
get_arg()
{
    for (l_arg = 0;; l_arg++)
        if (*(arg + l_arg) == '\n' ||
            *(arg + l_arg) == '\0' ||
            *(arg + l_arg) == ',' ||
            *(arg + l_arg) == ' ')
            break;
    for (s_arg = 0; *(arg + l_arg + s_arg) == ' ' ||
                    *(arg + l_arg + s_arg) == ',';
         s_arg++)
        ;
}
static int
get_det()
{
    register i;
    det_table = last_det;
    while (det_table != NULL)
    {
        if (strncmp(det_table->det_id, arg, l_arg) == 0)
        {
            if (*(det_table->det_id + l_arg) == '\0')
                return (1);
        }
        det_table = det_table->det_next;
    }
    if ((det_table = (DET_TAB *)malloc(sizeof(DET_TAB))) == NULL)
    {
    AB:
        wrfabe("No memory for parameter");
        return (0);
    }
    if ((det_table->det_id = (char *)malloc(l_arg + 1)) == NULL)
        goto AB;
    for (i = 0; i < l_arg; i++)
        *(det_table->det_id + i) = rfcnv(*(arg + i));
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
    return (1);
}
static int
g_numb(buffer, numb)
char *buffer;
long *numb;
{
    if (sscanf(buffer, "%ld", numb) == 0 || *numb < 0L)
    {
        if (buffer == reply_buf)
            msg_print("\nInvalid number; repeat please. ");
        return (0);
    }
    return (1);
}

static int
g_yn(buffer)
char *buffer;
{
    if (*buffer != 'y' && *buffer != 'n')
    {
        if (buffer == reply_buf)
            msg_print("\nAnswer is \"y/n\"; repeat please. ");
        return (0);
    }
    if (*buffer == 'y')
        e1empty = 1;
    else
        e1empty = 0;
    return (1);
}

static int
get_scr_parm(text)
char *text;
{
    int n;
    DET_TAB *d;
    def_window(WORK_MES, 0, 0, 79, 2, 0, 18, 79, 20, B_WHITE, SWAPABLE);
    draw_frame(0, 0, 79, 2, BLACK, SINGLE);
    n = strlen(text);
    inf.r_x = n + 2;
    inf.r_addr = text;
    if (f_alter < 8)
    {
        reply_buf[0] = '\0';
        for (d = last_det; d != NULL; d = d->det_next)
            if (*((char *)(&(d->gt)) + f_alter - 1) == 1)
            {
                strcat(reply_buf, d->det_id);
                strcat(reply_buf, " ");
                *((char *)(&(d->gt)) + f_alter - 1) = 0;
            }
    }
    m_read(&inf, &f1);
    if (ret_flag > 0)
    {
        ret_flag = 0;
        remove_window(WORK_MES);
        return (0);
    }
    if (f_alter < 8)
        print_parm(f_alter);
    remove_window(WORK_MES);
    return (1);
}

static int /* read debug task from screen */
rd_deb_screen()
{
    if (f_alter != 0)
        modif = '+'; /* !!! */
    switch (f_alter)
    {
    case 0:
        set_ret(1);
        return 0;
    case 1: /*    >    */
        if (get_scr_parm(" > (function list) : "))
            analize(reply_buf, 1);
        break;
    case 2: /*    >=   */
        if (get_scr_parm(" >= (function list) : "))
            analize(reply_buf, 2);
        break;
    case 3: /*     =   */
        if (get_scr_parm(" = (function list) : "))
            analize(reply_buf, 3);
        break;
    case 4: /*    !=   */
        if (get_scr_parm(" != (function list) : "))
            analize(reply_buf, 4);
        break;
    case 5: /*    <    */
        if (get_scr_parm(" < (function list) : "))
            analize(reply_buf, 5);
        break;
    case 6: /*    <=   */
        if (get_scr_parm(" <= (function list) : "))
            analize(reply_buf, 6);
        break;
    case 7: /*    TRAP  */
        if (get_scr_parm(" TRAP (function list) : "))
            analize(reply_buf, 7);
        break;
    case 8:
    R1:
        sprintf(reply_buf, "%ld", s_stop);
        if (get_scr_parm(" STOP (step number) : "))
        {
            if (*reply_buf != '\0')
                if (g_numb(reply_buf, &s_stop) == 0)
                    goto R1;
            print_parm(8);
        }
        break;
    case 9:
    R2:
        sprintf(reply_buf, "%ld", s_from);
        if (get_scr_parm(" FROM (step number) : "))
        {
            if (*reply_buf != '\0')
                if (g_numb(reply_buf, &s_from) == 0)
                    goto R2;
            print_parm(9);
        }
        break;
    case 10:
    R3:
        sprintf(reply_buf, "%ld", s_upto);
        if (get_scr_parm(" TO (step number) : "))
        {
            if (*reply_buf != '\0')
                if (g_numb(reply_buf, &s_upto) == 0)
                    goto R3;
            print_parm(10);
        }
        break;
    case 11:
    R4:
        if (e1empty)
            strcpy(reply_buf, "y");
        else
            strcpy(reply_buf, "n");
        if (get_scr_parm(" E1= (y/n) : "))
        {
            if (*reply_buf != '\0')
                if (g_yn(reply_buf) == 0)
                    goto R4;
            print_parm(11);
        }
    }
    set_window(MESSAGE);
    wputc('\n');
    return 0;
}

static void /* read debug task from file */
rd_deb_file()
{
    int i, j;
    char *pchar;
    while (fgets(buff, 100, dfile) != NULL)
    {
        for (j = 1; j <= 11; j++)
        {
            if (strncmp(buff, word[j], strlen(word[j])) == 0)
            {
                pchar = NULL;
                for (i = 0; i < strlen(buff); i++)
                    if (*(buff + i) == ':')
                        pchar = buff + i;
                if (pchar == NULL)
                {
                OSH:
                    sprintf(buff_id, "Error in debug line: %s", buff);
                    wrfabe(buff_id);
                    break;
                }
                if (!analize((char *)pchar + 1, j))
                    goto OSH;
                break;
            }
        }
        if (j == 12)
        {
            sprintf(buff_id, "Undefined debug line: %s", buff);
            wrfabe(buff_id);
            fclose(dfile);
        }
    }
}

static void /* write debug task to file */
wr_deb_file()
{
    int i;
    if ((wrfile = fopen("refal.cfg", "w")) == NULL)
    {
        wrfabe("Can't open file REFAL.CFG");
        return;
    }
    trace_cond = 0;
    ge_all = 1;
    eq_all = 1;
    for (i = 1; i <= 11; i++)
    {
        if (i == 8)
        {
            /*  set FROM and TO  */
            if (!s_from && (s_upto || trace_cond))
                s_from = 1;
            if (!s_upto && s_from)
                s_upto = 0x7FFFFFFFL;
        }
        if ((fputs(word[i], wrfile) == EOF) || !wr_det_numb(i))
        {
            wrfabe("Write error to file REFAL.CFG");
            fclose(wrfile);
            return;
        }
    }
    fclose(wrfile);
    return;
}

static int
wr_det_numb(altern)
int altern;
{
    DET_TAB *det_t;

    if (fputs(" : ", wrfile) == EOF)
        return (0);
    if (altern < 8)
    {
        for (det_t = last_det; det_t != NULL; det_t = det_t->det_next)
            if (*((char *)(&(det_t->gt)) + altern - 1) == 1)
            {
                trace_cond = 1;
                switch (altern)
                {
                case 1:
                case 2:
                    ge_all = 0;
                    break;
                case 3:
                    eq_all = 0;
                    break;
                default:
                    break;
                }
                if (fputs(det_t->det_id, wrfile) == EOF)
                    return (0);
                if (fputc(' ', wrfile) == EOF)
                    return (0);
            }
    }
    else
        switch (altern)
        {
        case 8:
            if (fprintf(wrfile, "%ld", s_stop) == EOF)
                return (0);
            break;
        case 9:
            if (fprintf(wrfile, "%ld", s_from) == EOF)
                return (0);
            break;
        case 10:
            if (fprintf(wrfile, "%ld", s_upto) == EOF)
                return (0);
            break;
        case 11:
            if (e1empty == 0)
            {
                if (fputc('n', wrfile) == EOF)
                    return (0);
                break;
            }
            else if (fputc('y', wrfile) == EOF)
                return (0);
            break;
        }
    if (fputc('\n', wrfile) == EOF)
        return (0);
    return (1);
}

static int
analize(buffer, altern)
int altern;
char *buffer;
{
    arg = buffer;
    while (*arg == ' ')
        arg++;
    switch (altern)
    {
    case 1: /*    >    */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            ge_all = 0;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->gt = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 2: /*   >=    */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            ge_all = 0;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->ge = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 3: /*    =    */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            eq_all = 0;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->eq = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 4: /*   !=  */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->ne = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 5: /*   <    */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->lt = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 6: /*   <=   */
        if (*arg != '\n' && *arg != '\0')
        {
            trace_cond = 1;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->le = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 7: /*  TRAP  */
        if (*arg != '\n' && *arg != '\0')
        {
            trap_cond = 1;
            while (*arg != '\n' && *arg != '\0')
            {
                get_arg();
                if (!get_det())
                    return (0);
                det_table->tr = 1;
                arg = arg + l_arg + s_arg;
            }
        }
        break;
    case 8:
        if (*arg != '\n' && *arg != '\0')
        {
            if (g_numb(arg, &s_stop) == 0)
                return (0);
        }
        else
            s_stop = 0l;
        break;
    case 9:
        if (*arg != '\n' && *arg != '\0')
        {
            if (g_numb(arg, &s_from) == 0)
                return (0);
        }
        else
            s_from = 0l;
        break;
    case 10:
        if (*arg != '\n' && *arg != '\0')
        {
            if (g_numb(arg, &s_upto) == 0)
                return (0);
        }
        else
            s_upto = 0l;
        break;
    case 11:
        if ((*arg != '\n' && *arg != '\0') && (*arg == 'y'))
            e1empty = 1;
    }
    return (1);
}
/*==================================*/
#else

static void get_arg()
{
    for (l_arg = 0;; l_arg++)
        if ((*(arg + l_arg) = rfcnv(*(arg + l_arg))) == '\n' || *(arg + l_arg) == ' ' || *(arg + l_arg) == '\0' || *(arg + l_arg) == ',')
            break;
    for (s_arg = 0; (*(arg + s_arg) == ' ') || (*(arg + s_arg) == ','); s_arg++)
        ;
    return;
}

static int get_det()
{
    register i;
    det_table = last_det;
    while (det_table != NULL)
    {
        if (strncmp(det_table->det_id, arg, l_arg) == 0)
        {
            if (*(det_table->det_id + l_arg) == '\0')
                return 1;
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
    for (i = 0; i < l_arg; i++)
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
    return 1;
}

static int get_numb(long *numb)
{
    if (sscanf(buff, "%ld", numb) == 0 || *numb < 1L)
    {
        printf("\n                        Invalid number; repeat please.");
        return 0;
    }
    return 1;
}

static int get_yn(char *b)
{
    if (*b != 'y' && *b != 'n')
    {
        printf("\n                        Answer is \"y/n\"; repeat please.");
        return 0;
    }
    if (*b == 'y')
        e1empty = 1;
    return 1;
}
#endif
/*---------  end of file DEBUG.C -----------*/
