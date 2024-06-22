#ifndef RFRUN2_H
#define RFRUN2_H

struct spcs;
struct ts;
struct wjs;

extern int cmpr(int n, unsigned char *p1, unsigned char *p2);
extern void link(linkcb *x, linkcb *y);
extern void move(int n, unsigned char *pf, unsigned char *pt);
extern int spc(struct spcs *pspcsp, char *vpc, linkcb *b);

extern void getjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, int *anel, char **avpc);
extern void putjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, int *anel, char **avpc);
extern void getts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az);
extern void putts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az);

#endif