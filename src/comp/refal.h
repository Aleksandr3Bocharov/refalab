#ifndef REFAL_H
#define REFAL_H

#include "cgop.h"

extern struct
{ /* the table for corresponding with scanner */
    int nomkar;
    char modname_var[40]; /* module name */ /*  !!! */
    int modnmlen;                           /* module name length */
    int curr_stmnmb;
} scn_;

extern struct
{                         /* current statement element */
    unsigned short int t; /*    element type           */
    char ci;              /*    variable index         */
    int v;
    struct linkti code;
    struct linkti spec;
} scn_e;

extern void oshibka();
extern void scan();

#endif