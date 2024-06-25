#ifndef RFRUN2_H
#define RFRUN2_H

struct wjs
{ /* jump stack structure */
    linkcb *jsb1;
    linkcb *jsb2;
    unsigned int jsnel;
    unsigned char *jsvpc;
};

struct ts
{ /* translation stack structure*/
    linkcb *ts0;
    linkcb *ts1;
    linkcb *ts2;
};

struct spcs
{
    unsigned int spls;
    unsigned char *svpc;
};

extern unsigned int cmpr(unsigned int n, unsigned char *p1, unsigned char *p2);
extern void link(linkcb *x, linkcb *y);
extern void move(unsigned int n, unsigned char *pf, unsigned char *pt);
extern unsigned int spc(struct spcs *pspcsp, unsigned char *vpc, linkcb *b);

extern void getjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, unsigned int *anel, unsigned char **avpc);
extern void putjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, unsigned int *anel, unsigned char **avpc);
extern void getts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az);
extern void putts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az);

#endif