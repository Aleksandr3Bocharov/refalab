#ifndef CJ_H
#define CJ_H

#include <stdio.h>
#include "clu.h"

extern void j3addr(T_U *pp);
extern void jbyte(char bb);
extern void jend();
extern void jentry(T_U *pp, const char *ee, unsigned int ll);
extern void jequ(T_U *pp, T_U *qq);
extern void jextrn(T_U *pp, const char *ee, unsigned int ll);
extern void jlabel(T_U *pp);
extern void jstart(const char *ee, unsigned int ll);
extern size_t jwhere();

#endif