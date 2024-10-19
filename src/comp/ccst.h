// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------   file  CCST.H  ---------------
//      Compiler file
//------------------------------------------

#ifndef CCST_H
#define CCST_H

#include <stddef.h>
#include <stdbool.h>

// the table of assembly language statement codes  
#define n_sjump '\001'
#define n_fail '\002'
#define n_sb1b2 '\003'
#define n_lsc '\004'
#define n_rsc '\005'
#define n_lsco '\006'
#define n_rsco '\007'
#define n_lsd '\010'
#define n_rsd '\011'
#define n_ltxt '\012'
#define n_rtxt '\013'
#define n_lb '\014'
#define n_lby '\015'
#define n_rb '\016'
#define n_rby '\017'
#define n_ls '\020'
#define n_rs '\021'
#define n_lw '\022'
#define n_rw '\023'
#define n_lbnil '\024'
#define n_rbnil '\025'
#define n_lbce '\026'
#define n_rbce '\027'
#define n_nil '\030'
#define n_ce '\031'
#define n_led '\032'
#define n_red '\033'
#define n_nnil '\034'
#define n_ple '\035'
#define n_plv '\036'
#define n_le '\037'
#define n_pre '\040'
#define n_prv '\041'
#define n_re '\042'
#define n_plesc '\043'
#define n_plvsc '\044'
#define n_lesc '\045'
#define n_presc '\046'
#define n_prvsc '\047'
#define n_resc '\050'
#define n_lesd '\051'
#define n_resd '\052'
#define n_pleb '\053'
#define n_plvb '\054'
#define n_leb '\055'
#define n_preb '\056'
#define n_prvb '\057'
#define n_reb '\060'
#define n_eoe '\061'
#define n_eoei '\062'
#define n_lsrch '\063'
#define n_rsrch '\064'
#define n_wspc '\065'
#define n_espc '\066'
#define n_plespc '\067'
#define n_lespc '\070'
#define n_prespc '\071'
#define n_respc '\072'
#define n_lmax '\073'
#define n_rmax '\074'
#define n_eor '\075'
#define n_ns '\076'
#define n_nso '\077'
#define n_text '\100'
#define n_bl '\101'
#define n_br '\102'
#define n_blr '\103'
#define n_bract '\104'
#define n_act '\105'
#define n_muls '\106'
#define n_mule '\107'
#define n_tpl '\110'
#define n_tplm '\111'
#define n_tple '\112'
#define n_tplv '\113'
#define n_tpls '\114'
#define n_eos '\115'
#define n_swap '\116'
#define n_blf '\117'
//#define n_eossn '\120'
//#define n_setnos '\121'

extern void cst(bool dir, char *lbl, size_t lblleng);

#endif
