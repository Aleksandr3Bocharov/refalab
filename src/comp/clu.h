#ifndef CLU_H
#define CLU_H

struct refw
{
    struct refw *next; /* on the next usage list */
    int numb[6];       /* usage list element     */
};

struct u
{
    union
    {
        int infon;
        struct u *infop;
    } info;
    char mode;
    /* mode field :                      */
    /*  00 - no defined;                          */
    /*  01 - internal; infon = offset from start  */
    /*  10 - external; infon = member or extern   */
    /*       reference;                           */
    /*  11 - equivalent; infop =  reference on    */
    /*       other label;                         */
    /*  xx1 - entry point;                        */
    /*  xxx1 - too many definition;               */
    /*                                            */
    char type;             /* type field : 00 - unknown type  */
                           /*              01 - function      */
                           /*              10 - specifier     */
    int l;                 /* identifier length */
    struct u *i;           /* left reference */
    struct u *j;           /* right reference */
    struct refw *last_ref; /* on the end of using list */
    struct refw ref;       /* where used */
    int def;               /* where defined */
    char k;                /* kren feature:      '00'B - kren no    */
                           /*                    '01'B - left kren  */
                           /*                    '10'B - right kren */
    char *id;              /* identifier */
};
typedef struct u T_U;

extern struct u *lookup(char *idp, int lid);
extern void luterm();
extern void through(void (*prog)(struct u *));
extern void uns_sto();

#endif