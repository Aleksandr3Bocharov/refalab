#ifndef CLU_H
#define CLU_H

extern struct u *lookup(char *idp, int lid);
extern void through(void (*prog)(struct u *));

#endif