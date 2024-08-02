#ifndef REFAL_H
#define REFAL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cgop.h"

typedef struct opt_
{ // compiling   options  
    unsigned int source : 1;
    unsigned int mincomp : 1;
    unsigned int stmnmb : 1;
    unsigned int extname : 1;
    unsigned int multmod : 1;
    unsigned int names : 1;
} T_OPT;

extern T_OPT options;

typedef struct scn_
{ // the table for corresponding with scanner  
    unsigned int nomkar;
    char modname_var[40];  // module name  
    unsigned int modnmlen; // module name length  
    unsigned int curr_stmnmb;
} T_SCN;

extern T_SCN scn_;

typedef struct scn_e_
{                         // current statement element  
    uint16_t t; //    element type            
    char ci;              //    variable index          
    bool v;
    T_LINKTI code;
    T_LINKTI spec;
} T_SCN_E;

extern T_SCN_E scn_e;

extern FILE *sysprint, *systerm;
extern FILE *syslin; // for assem  
extern FILE *systxt; // for module names  

extern uint16_t nommod;
extern char parm_i[];       // sourse file name  
extern const char vers_i[]; // compiler version  
extern char mod_i[];        // module name       

extern void oshibka();
extern void scan();

#endif