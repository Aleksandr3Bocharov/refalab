#ifndef REFAL_H
#define REFAL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cgop.h"

typedef struct opt_
{ // compiling   options
    bool source;
    bool mincomp;
    bool stmnmb;
    bool extname;
    bool multmod;
    bool names;
} T_OPT;

extern T_OPT options;

typedef struct scn_
{ // the table for corresponding with scanner
    uint32_t nomkar;
    char modname_var[40]; // module name
    size_t modnmlen;      // module name length
    //    uint16_t curr_stmnmb;
} T_SCN;

extern T_SCN scn_;

typedef struct scn_e_
{               // current statement element
    uint32_t t; //    element type
    char ci;    //    variable index
    bool v;
    T_LINKTI code;
    T_LINKTI spec;
} T_SCN_E;

extern T_SCN_E scn_e;

extern FILE *sysprint, *systerm;
extern FILE *syslin; // for assem
extern FILE *systxt; // for module names

extern uint32_t nommod;
extern char parm_i[];       // sourse file name
extern const char vers_i[]; // compiler version
extern char mod_i[];        // module name

extern void oshibka();
extern void scan();

#endif