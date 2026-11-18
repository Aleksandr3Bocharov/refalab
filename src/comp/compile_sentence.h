// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compile_sentence.h  ----------
//   Compilation of a sentence: left and right part
//------------------------------------------------

#ifndef COMPILER_SENTENCE_H
#define COMPILER_SENTENCE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// the table of assembly language statement codes
// The table of assembly language statement codes (octal). Naming of the statements:
//   l.../r...   - projection from the left / from the right board of a hole
//   s, w, e     - s-, w- and e-variable (a symbol, a term, an expression)
//   ...d        - the variable is repeated: compare with its first occurrence (n_lsd, n_led, ...)
//   sc / sco    - a constant symbol / a constant character
//   txt         - a text of several characters
//   b, nil, ce  - brackets, an empty hole, a closed e-variable
//   ple/plv, pre/prv - save the point to return to when an e-/v-variable is expanded
//   le/re       - expand an e-variable by one term; ...sc, ...sd, ...b - expand up to a symbol,
//                 a repeated s-variable or a bracket
//   lsrch/rsrch, eoe/eoei - search for a symbol; end of a search level
//   wspc/espc.. - check of the type specifier; lmax/rmax - maximal projection
//   eor         - end of the left part (recognition)
//   ns, nso, text, bl, br, blr, blf, bract, muls, mule - building of the result
//   tpl, tplm, tple, tplv, tpls - transplantation of a piece of the left part
//   eos         - end of the sentence; sjump, fail, swap - control statements and a box
#define n_sjump 0001
#define n_fail 0002
#define n_sb1b2 0003
#define n_lsc 0004
#define n_rsc 0005
#define n_lsco 0006
#define n_rsco 0007
#define n_lsd 0010
#define n_rsd 0011
#define n_ltxt 0012
#define n_rtxt 0013
#define n_lb 0014
#define n_lby 0015
#define n_rb 0016
#define n_rby 0017
#define n_ls 0020
#define n_rs 0021
#define n_lw 0022
#define n_rw 0023
#define n_lbnil 0024
#define n_rbnil 0025
#define n_lbce 0026
#define n_rbce 0027
#define n_nil 0030
#define n_ce 0031
#define n_led 0032
#define n_red 0033
#define n_nnil 0034
#define n_ple 0035
#define n_plv 0036
#define n_le 0037
#define n_pre 0040
#define n_prv 0041
#define n_re 0042
#define n_plesc 0043
#define n_plvsc 0044
#define n_lesc 0045
#define n_presc 0046
#define n_prvsc 0047
#define n_resc 0050
#define n_lesd 0051
#define n_resd 0052
#define n_pleb 0053
#define n_plvb 0054
#define n_leb 0055
#define n_preb 0056
#define n_prvb 0057
#define n_reb 0060
#define n_eoe 0061
#define n_eoei 0062
#define n_lsrch 0063
#define n_rsrch 0064
#define n_wspc 0065
#define n_espc 0066
#define n_plespc 0067
#define n_lespc 0070
#define n_prespc 0071
#define n_respc 0072
#define n_lmax 0073
#define n_rmax 0074
#define n_eor 0075
#define n_ns 0076
#define n_nso 0077
#define n_text 0100
#define n_bl 0101
#define n_br 0102
#define n_blr 0103
#define n_bract 0104
#define n_muls 0106
#define n_mule 0107
#define n_tpl 0110
#define n_tplm 0111
#define n_tple 0112
#define n_tplv 0113
#define n_tpls 0114
#define n_eos 0115
#define n_swap 0116
#define n_blf 0117

// Types of the sentence elements returned by the scanner:
// SC - a constant symbol, LB/RB - parentheses, S_V/W_V/E_V - variables,
// K/DOT - call brackets '<' and '>', EQ - the sign '=', END - the end of the sentence,
// NONE - an error of the scanner.
typedef enum sentence_element_types
{
    NONE,
    SC,
    LB,
    RB,
    S_V,
    W_V,
    E_V,
    K,
    DOT,
    EQ,
    END
} T_SENTENCE_ELEMENT_TYPES;

// Compiles one sentence; direction: true - matching from left to right.
extern void compile_sentence(bool direction);

#endif
