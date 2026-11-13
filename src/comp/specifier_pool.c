// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-14
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file specifier_pool.c  -----------
//    pool of unique specifiers optimization
//----------------------------------------------

#if defined mdebug
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <ctype.h>
#include "refalab.h"
#include "specifier_pool.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "print_errors.h"
#include "avl_identifiers.h"
#include "identifiers.h"

static T_UNIQUE_SPECIFIER *pool_head = NULL;

static uint8_t *spec_buffer = NULL;
static size_t spec_buffer_size = 0;
static size_t spec_buffer_capacity = 0;
static bool collecting = false;

static T_PENDING_ADDRESS *pending_addresses = NULL;
static size_t pending_count = 0;
static size_t pending_capacity = 0;

void specifier_pool_init(void)
{
    pool_head = NULL;
    spec_buffer = NULL;
    spec_buffer_size = 0;
    spec_buffer_capacity = 0;
    collecting = false;
    pending_addresses = NULL;
    pending_count = 0;
    pending_capacity = 0;
}

void specifier_pool_clear(void)
{
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        T_UNIQUE_SPECIFIER *next = current->next;
#if defined mdebug
        fprintf(stderr, "free(specifier_pool_clear): current=%p bytes=%p addresses=%p\n", (void *)current, (void *)current->bytes, (void *)current->addresses);
#endif
        free(current->bytes);
        if (current->addresses != NULL)
            free(current->addresses);
        free(current);
        current = next;
    }
    pool_head = NULL;
#if defined mdebug
    fprintf(stderr, "free(specifier_pool_clear): spec_buffer=%p pending_addresses=%p\n", (void *)spec_buffer, (void *)pending_addresses);
#endif
    free(spec_buffer);
    spec_buffer = NULL;
    spec_buffer_size = 0;
    spec_buffer_capacity = 0;
    free(pending_addresses);
    pending_addresses = NULL;
    pending_count = 0;
    pending_capacity = 0;
    collecting = false;
    return;
}

bool specifier_pool_is_collecting(void)
{
    return collecting;
}

void specifier_buffer_begin(void)
{
    collecting = true;
    spec_buffer_size = 0;
    pending_count = 0;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_begin: collecting=true\n");
#endif
}

static void ensure_buffer_capacity(size_t needed)
{
    if (spec_buffer_capacity < needed)
    {
        size_t new_capacity = spec_buffer_capacity == 0 ? 64 : spec_buffer_capacity * 2;
        while (new_capacity < needed)
            new_capacity *= 2;
        uint8_t *new_buffer = realloc(spec_buffer, new_capacity);
        if (new_buffer == NULL)
            error_no_memory();
        spec_buffer = new_buffer;
        spec_buffer_capacity = new_capacity;
#if defined mdebug
        fprintf(stderr, "realloc(ensure_buffer_capacity): spec_buffer=%p spec_buffer_capacity=%zu\n", (void *)spec_buffer, spec_buffer_capacity);
#endif
    }
}

void specifier_buffer_append_byte(uint8_t byte)
{
    if (!collecting)
        return;
    ensure_buffer_capacity(spec_buffer_size + 1);
    spec_buffer[spec_buffer_size++] = byte;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_append_byte: byte=0x%02X size=%zu\n", byte, spec_buffer_size);
#endif
}

void specifier_buffer_append_address(T_LABEL *label)
{
    if (!collecting)
        return;
    if (pending_count >= pending_capacity)
    {
        size_t new_capacity = pending_capacity == 0 ? 8 : pending_capacity * 2;
        T_PENDING_ADDRESS *new_pending = realloc(pending_addresses, new_capacity * sizeof(T_PENDING_ADDRESS));
        if (new_pending == NULL)
            error_no_memory();
        pending_addresses = new_pending;
        pending_capacity = new_capacity;
#if defined mdebug
        fprintf(stderr, "realloc(specifier_buffer_append_address): pending_addresses=%p pending_capacity=%zu\n", (void *)pending_addresses, pending_capacity);
#endif
    }
    pending_addresses[pending_count].offset = spec_buffer_size;
    pending_addresses[pending_count].label = label;
    pending_count++;
    ensure_buffer_capacity(spec_buffer_size + LBLL);
    memset(spec_buffer + spec_buffer_size, 0, LBLL);
    spec_buffer_size += LBLL;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_append_address: label=%p offset=%zu size=%zu\n", (void *)label, spec_buffer_size - LBLL, spec_buffer_size);
#endif
}

void specifier_buffer_append_symbol(const T_LINKTI *code)
{
    if (!collecting)
        return;
    const uint8_t *tag_bytes = (const uint8_t *)&(code->tag);
    ensure_buffer_capacity(spec_buffer_size + ZBLL);
    memcpy(spec_buffer + spec_buffer_size, tag_bytes, ZBLL);
    spec_buffer_size += ZBLL;
    if (code->tag == TAGF)
    {
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGF codef=%p\n", (void *)code->info.codef);
#endif
        specifier_buffer_append_address(code->info.codef);
        return;
    }
    ensure_buffer_capacity(spec_buffer_size + LBLL);
    if (code->tag == TAGO)
    {
        spec_buffer[spec_buffer_size] = code->info.infoc;
        memset(spec_buffer + spec_buffer_size + 1, 0, LBLL - 1);
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGO infoc=0x%02X\n", (unsigned char)code->info.infoc);
#endif
    }
    else
    {
        const uint8_t *info_bytes = (const uint8_t *)&(code->info.coden);
        memcpy(spec_buffer + spec_buffer_size, info_bytes, ZBLL);
        memset(spec_buffer + spec_buffer_size + ZBLL, 0, LBLL - ZBLL);
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGN coden=%" PRIu32 "\n", code->info.coden);
#endif
    }
    spec_buffer_size += LBLL;
}

// ---------------------------------------------------------------
//  canonical form of specifier
//
//  Specifier is a chain of segments divided by ns_ng:
//     S0 ns_ng S1 ns_ng S2 ... ns_ngw
//  The first term of the chain that matches a link decides the answer:
//  "yes" for an even segment, "no" for an odd one; if nothing matches
//  the answer is the opposite of the parity of the last segment.
//  So inside one segment the order of terms is not important, and a term
//  covered by another term of the same or of an earlier segment is dead.
//  The canonical form (sorted terms, no dead terms, nothing after "w")
//  makes more specifiers identical, and "w" in the first segment means
//  "any link": such a specifier needs no check at all.
// ---------------------------------------------------------------

#define SP_CLL 0
#define SP_W 1
#define SP_NG 2
#define SP_NGW 3
#define SP_SC 4
#define SP_S 5
#define SP_B 6
#define SP_F 7
#define SP_N 8
#define SP_R 9
#define SP_O 10
#define SP_D 11
#define SP_L 12
#define SP_RN 13
#define SP_RC 14

typedef struct sp_term
{
    uint8_t type;    // term code
    size_t seg;      // segment number
    size_t offset;   // offset of the payload in spec_buffer
    size_t length;   // payload length
    size_t addr_rel; // offset of the address inside the payload
    T_LABEL *label;  // address label (cll or TAGF symbol)
    bool dead;
} T_SP_TERM;

static bool parse_terms(const uint8_t *buf, size_t size, const T_PENDING_ADDRESS *pend, size_t pcount, T_SP_TERM *terms, size_t *terms_count, size_t *segments_count)
{
    size_t pos = 0;
    size_t count = 0;
    size_t seg = 0;
    size_t addr = 0;
    bool closed = false;
    while (pos < size)
    {
        if (closed)
            return false;
        const uint8_t op = buf[pos++];
        size_t payload = 0;
        bool has_addr = false;
        size_t addr_off = 0;
        switch (op)
        {
        case SP_NG:
            seg++;
            continue;
        case SP_NGW:
            closed = true;
            continue;
        case SP_W:
        case SP_S:
        case SP_B:
        case SP_F:
        case SP_N:
        case SP_R:
        case SP_O:
        case SP_D:
        case SP_L:
            break;
        case SP_CLL:
            payload = LBLL;
            has_addr = true;
            addr_off = pos;
            break;
        case SP_SC:
        {
            payload = SMBL;
            if (pos + ZBLL > size)
                return false;
            uint32_t tag;
            memcpy(&tag, buf + pos, ZBLL);
            if (tag == TAGF)
            {
                has_addr = true;
                addr_off = pos + ZBLL;
            }
            break;
        }
        case SP_RN:
            payload = 2 * ZBLL;
            break;
        case SP_RC:
            payload = 2;
            break;
        default:
            return false;
        }
        if (pos + payload > size)
            return false;
        T_LABEL *label = NULL;
        if (has_addr)
        {
            if (addr >= pcount || pend[addr].offset != addr_off)
                return false;
            label = pend[addr].label;
            addr++;
        }
        terms[count].type = op;
        terms[count].seg = seg;
        terms[count].offset = pos;
        terms[count].length = payload;
        terms[count].addr_rel = has_addr ? addr_off - pos : 0;
        terms[count].label = label;
        terms[count].dead = false;
        count++;
        pos += payload;
    }
    if (!closed || addr != pcount)
        return false;
    *terms_count = count;
    *segments_count = seg + 1;
    return true;
}

static bool parse_buffer_terms(T_SP_TERM *terms, size_t *terms_count, size_t *segments_count)
{
    return parse_terms(spec_buffer, spec_buffer_size, pending_addresses, pending_count, terms, terms_count, segments_count);
}

static inline uint32_t sc_tag(const T_SP_TERM *t)
{
    uint32_t tag;
    memcpy(&tag, spec_buffer + t->offset, ZBLL);
    return tag;
}

static inline uint32_t term_u32(const T_SP_TERM *t, size_t index)
{
    uint32_t value;
    memcpy(&value, spec_buffer + t->offset + index * ZBLL, ZBLL);
    return value;
}

static bool terms_equal(const T_SP_TERM *a, const T_SP_TERM *b)
{
    return a->type == b->type && a->length == b->length && a->label == b->label &&
           memcmp(spec_buffer + a->offset, spec_buffer + b->offset, a->length) == 0;
}

// true if every link matched by b is matched by a
static bool term_covers(const T_SP_TERM *a, const T_SP_TERM *b)
{
    if (terms_equal(a, b))
        return true;
    switch (a->type)
    {
    case SP_W:
        return true;
    case SP_S:
        switch (b->type)
        {
        case SP_F:
        case SP_N:
        case SP_R:
        case SP_O:
        case SP_D:
        case SP_L:
        case SP_RN:
        case SP_RC:
            return true;
        case SP_SC:
            return (sc_tag(b) & 1) == 0;
        default:
            return false;
        }
    case SP_O:
        switch (b->type)
        {
        case SP_D:
        case SP_L:
        case SP_RC:
            return true;
        case SP_SC:
            return sc_tag(b) == TAGO;
        default:
            return false;
        }
    case SP_N:
        if (b->type == SP_RN)
            return true;
        return b->type == SP_SC && sc_tag(b) == TAGN;
    case SP_F:
        return b->type == SP_SC && sc_tag(b) == TAGF;
    case SP_R:
        return b->type == SP_SC && sc_tag(b) == TAGR;
    case SP_RC:
    {
        const uint8_t first = spec_buffer[a->offset];
        const uint8_t second = spec_buffer[a->offset + 1];
        if (b->type == SP_RC)
            return first <= spec_buffer[b->offset] && spec_buffer[b->offset + 1] <= second;
        if (b->type == SP_SC && sc_tag(b) == TAGO)
        {
            const uint8_t symbol = spec_buffer[b->offset + ZBLL];
            return symbol >= first && symbol <= second;
        }
        return false;
    }
    case SP_RN:
    {
        const uint32_t first = term_u32(a, 0);
        const uint32_t second = term_u32(a, 1);
        if (b->type == SP_RN)
            return first <= term_u32(b, 0) && term_u32(b, 1) <= second;
        if (b->type == SP_SC && sc_tag(b) == TAGN)
        {
            const uint32_t number = term_u32(b, 1); // after the tag
            return number >= first && number <= second;
        }
        return false;
    }
    default:
        return false;
    }
}

static int term_rank(uint8_t type)
{ // cheap and general checks first, calls of other specifiers last
    switch (type)
    {
    case SP_W:
        return 0;
    case SP_S:
        return 1;
    case SP_B:
        return 2;
    case SP_F:
        return 3;
    case SP_N:
        return 4;
    case SP_R:
        return 5;
    case SP_O:
        return 6;
    case SP_D:
        return 7;
    case SP_L:
        return 8;
    case SP_RN:
        return 9;
    case SP_RC:
        return 10;
    case SP_SC:
        return 11;
    default:
        return 12;
    }
}

static int label_compare(const T_LABEL *a, const T_LABEL *b)
{ // only labels with identifiers: functions and named specifiers
    if (a == b)
        return 0;
    if (a == NULL)
        return -1;
    if (b == NULL)
        return 1;
    const uint8_t length = a->identifier_length < b->identifier_length ? a->identifier_length : b->identifier_length;
    const int result = memcmp(a->identifier, b->identifier, length);
    if (result != 0)
        return result;
    if (a->identifier_length != b->identifier_length)
        return a->identifier_length < b->identifier_length ? -1 : 1;
    return 0;
}

static int term_compare(const void *pa, const void *pb)
{
    const T_SP_TERM *a = (const T_SP_TERM *)pa;
    const T_SP_TERM *b = (const T_SP_TERM *)pb;
    if (a->seg != b->seg)
        return a->seg < b->seg ? -1 : 1;
    const int rank_a = term_rank(a->type);
    const int rank_b = term_rank(b->type);
    if (rank_a != rank_b)
        return rank_a < rank_b ? -1 : 1;
    const int result = memcmp(spec_buffer + a->offset, spec_buffer + b->offset, a->length);
    if (result != 0)
        return result;
    return label_compare(a->label, b->label);
}

// returns false if the specifier matches any link (no check is needed)
static bool canonicalize_buffer(void)
{
    if (spec_buffer_size == 0)
        return true;
    T_SP_TERM *terms = malloc(spec_buffer_size * sizeof(T_SP_TERM));
    if (terms == NULL)
        error_no_memory();
    size_t terms_count = 0;
    size_t segments_count = 0;
    if (!parse_buffer_terms(terms, &terms_count, &segments_count))
    {
        free(terms);
        return true;
    }
    // "s" and "b" in one segment together are "w"
    for (size_t i = 0; i < terms_count; i++)
        if (terms[i].type == SP_S && !terms[i].dead)
            for (size_t j = 0; j < terms_count; j++)
                if (terms[j].type == SP_B && !terms[j].dead && terms[j].seg == terms[i].seg)
                {
                    terms[i].type = SP_W;
                    terms[i].length = 0;
                    terms[j].dead = true;
                    break;
                }
    // dead terms: covered by a term of the same or of an earlier segment
    for (size_t i = 0; i < terms_count; i++)
    {
        if (terms[i].dead)
            continue;
        for (size_t j = 0; j < terms_count; j++)
        {
            if (j == i || terms[j].dead || terms[j].seg > terms[i].seg)
                continue;
            if (!term_covers(&terms[j], &terms[i]))
                continue;
            if (term_covers(&terms[i], &terms[j]) && terms[j].seg == terms[i].seg && j > i)
                continue; // equal terms in one segment: the first one stays
            terms[i].dead = true;
            break;
        }
    }
    // everything after "w" is dead
    size_t limit = segments_count - 1;
    for (size_t i = 0; i < terms_count; i++)
        if (!terms[i].dead && terms[i].type == SP_W && terms[i].seg < limit)
            limit = terms[i].seg;
    for (size_t i = 0; i < terms_count; i++)
        if (!terms[i].dead && terms[i].type == SP_W && terms[i].seg == 0)
        {
            free(terms);
            return false;
        }
    qsort(terms, terms_count, sizeof(T_SP_TERM), term_compare);
    uint8_t *new_buffer = malloc(spec_buffer_size);
    T_PENDING_ADDRESS *new_pending = malloc((pending_count + 1) * sizeof(T_PENDING_ADDRESS));
    if (new_buffer == NULL || new_pending == NULL)
        error_no_memory();
    size_t new_size = 0;
    size_t new_pending_count = 0;
    size_t current_seg = 0;
    for (size_t i = 0; i < terms_count; i++)
    {
        if (terms[i].dead || terms[i].seg > limit)
            continue;
        while (current_seg < terms[i].seg)
        {
            new_buffer[new_size++] = SP_NG;
            current_seg++;
        }
        new_buffer[new_size++] = terms[i].type;
        if (terms[i].length > 0)
            memcpy(new_buffer + new_size, spec_buffer + terms[i].offset, terms[i].length);
        if (terms[i].label != NULL)
        {
            new_pending[new_pending_count].offset = new_size + terms[i].addr_rel;
            new_pending[new_pending_count].label = terms[i].label;
            new_pending_count++;
        }
        new_size += terms[i].length;
    }
    while (current_seg < limit)
    {
        new_buffer[new_size++] = SP_NG;
        current_seg++;
    }
    new_buffer[new_size++] = SP_NGW;
    memcpy(spec_buffer, new_buffer, new_size);
    spec_buffer_size = new_size;
    if (new_pending_count > 0)
        memcpy(pending_addresses, new_pending, new_pending_count * sizeof(T_PENDING_ADDRESS));
    pending_count = new_pending_count;
    free(new_buffer);
    free(new_pending);
    free(terms);
    return true;
}

// ---------------------------------------------------------------
//  full minimization of specifier
//
//  A specifier is a function F: link -> {yes, no}. The links are divided
//  into a finite number of atoms (the links inside an atom are not
//  distinguishable by the specifier): brackets, references, functions
//  (one atom for each function name used as a symbol and one for the others),
//  intervals of numbers and runs of characters. Calls of other specifiers
//  (:Name:) are black boxes: independent boolean variables c_j.
//  F is computed for every (atom, c) and then the shortest chain of
//  segments with the same F is searched for. The result is checked
//  against the original on all kinds of links; if anything differs
//  the original is kept.
// ---------------------------------------------------------------

#define MZ_MAX_CLL 4
#define MZ_MAX_LABELS 32
#define MZ_MAX_ATOMS 700
#define MZ_MAX_SEGS 5
#define MZ_MAX_TERMS 128
#define MZ_MAX_RUNS 64
#define MZ_WORK_LIMIT 30000000UL

#define MZ_C_RC 3
#define MZ_C_RN (1 + 2 * ZBLL)
#define MZ_C_SCF (1 + SMBL)
#define MZ_C_CLL (1 + LBLL)

enum
{
    MZ_B,
    MZ_R,
    MZ_F,
    MZ_N,
    MZ_O
};

typedef struct mz_atom
{
    uint8_t group;
    uint32_t lo;
    uint32_t hi;
    T_LABEL *label;
    bool digit;
    bool alpha;
    uint16_t out;  // bit c: the answer is "yes" for the vector c of black boxes
    uint16_t ccov; // bit k: the class k covers the atom
} MZ_ATOM;

typedef struct mz_term
{
    uint8_t type;
    uint32_t first;
    uint32_t second;
    T_LABEL *label;
    uint8_t seg;
} MZ_TERM;

typedef struct mz_probe
{
    uint32_t tag;
    uint32_t val;
    T_LABEL *label;
} MZ_PROBE;

static const uint8_t mz_class_code[9] = {SP_W, SP_S, SP_B, SP_F, SP_N, SP_R, SP_O, SP_D, SP_L};

static MZ_ATOM mz_atoms[MZ_MAX_ATOMS];
static size_t mz_natoms;
static T_LABEL *mz_rl[MZ_MAX_CLL];
static size_t mz_m;
static uint16_t mz_full;
static uint16_t mz_cmask[MZ_MAX_CLL];
static MZ_TERM mz_cur[MZ_MAX_TERMS];
static MZ_TERM mz_best[MZ_MAX_TERMS];
static size_t mz_cur_n;
static size_t mz_best_n;
static size_t mz_best_segs;
static uint32_t mz_best_cost;
static unsigned long mz_work;

static bool mz_class_covers(unsigned k, const MZ_ATOM *a)
{
    switch (k)
    {
    case 0:
        return true;
    case 1:
        return a->group != MZ_B;
    case 2:
        return a->group == MZ_B;
    case 3:
        return a->group == MZ_F;
    case 4:
        return a->group == MZ_N;
    case 5:
        return a->group == MZ_R;
    case 6:
        return a->group == MZ_O;
    case 7:
        return a->group == MZ_O && a->digit;
    default:
        return a->group == MZ_O && a->alpha;
    }
}

static bool mz_term_matches(const uint8_t *buf, const T_SP_TERM *t, const MZ_PROBE *x, uint32_t cvec, T_LABEL *const *cl, size_t ncl)
{
    const uint8_t *pl = buf + t->offset;
    switch (t->type)
    {
    case SP_W:
        return true;
    case SP_S:
        return (x->tag & 1) == 0;
    case SP_B:
        return (x->tag & 1) != 0;
    case SP_F:
        return x->tag == TAGF;
    case SP_N:
        return x->tag == TAGN;
    case SP_R:
        return x->tag == TAGR;
    case SP_O:
        return x->tag == TAGO;
    case SP_D:
        return x->tag == TAGO && isdigit((int)(unsigned char)x->val);
    case SP_L:
        return x->tag == TAGO && isalpha((int)(unsigned char)x->val);
    case SP_RN:
    {
        uint32_t first;
        uint32_t second;
        memcpy(&first, pl, ZBLL);
        memcpy(&second, pl + ZBLL, ZBLL);
        return x->tag == TAGN && x->val >= first && x->val <= second;
    }
    case SP_RC:
        return x->tag == TAGO && x->val >= pl[0] && x->val <= pl[1];
    case SP_SC:
    {
        uint32_t tag;
        memcpy(&tag, pl, ZBLL);
        if (x->tag != tag)
            return false;
        if (tag == TAGO)
            return x->val == pl[ZBLL];
        if (tag == TAGN)
        {
            uint32_t number;
            memcpy(&number, pl + ZBLL, ZBLL);
            return x->val == number;
        }
        if (tag == TAGF)
            return x->label == t->label;
        return false;
    }
    case SP_CLL:
        for (size_t j = 0; j < ncl; j++)
            if (cl[j] == t->label)
                return ((cvec >> j) & 1) != 0;
        return false;
    default:
        return false;
    }
}

static bool mz_eval(const uint8_t *buf, const T_SP_TERM *terms, size_t n, size_t segs, const MZ_PROBE *x, uint32_t cvec, T_LABEL *const *cl, size_t ncl)
{
    size_t first = segs;
    for (size_t i = 0; i < n; i++)
        if (terms[i].seg < first && mz_term_matches(buf, &terms[i], x, cvec, cl, ncl))
            first = terms[i].seg;
    if (first < segs)
        return (first & 1) == 0;
    return ((segs - 1) & 1) != 0;
}

static bool mz_insert_label(T_LABEL **array, size_t *count, size_t capacity, T_LABEL *label)
{
    size_t pos = 0;
    while (pos < *count && label_compare(array[pos], label) < 0)
        pos++;
    if (pos < *count && label_compare(array[pos], label) == 0)
        return true;
    if (*count >= capacity)
        return false;
    for (size_t i = *count; i > pos; i--)
        array[i] = array[i - 1];
    array[pos] = label;
    (*count)++;
    return true;
}

static int mz_u32_compare(const void *pa, const void *pb)
{
    const uint32_t a = *(const uint32_t *)pa;
    const uint32_t b = *(const uint32_t *)pb;
    return a < b ? -1 : (a > b ? 1 : 0);
}

// the cheapest set of terms of one segment that covers all "must" elements and
// touches no "forbidden" one; classes and black boxes are given as masks
static bool mz_cover_eval(const uint16_t *must, const uint16_t *forb, unsigned classes, unsigned cllset,
                          MZ_TERM *out, size_t *nout, uint32_t *cost_out, uint32_t limit)
{
    uint32_t cost = 0;
    size_t count = 0;
    for (unsigned c = 0; c < 9; c++)
        if ((classes >> c) & 1)
        {
            cost++;
            if (out != NULL)
            {
                out[count].type = mz_class_code[c];
                out[count].first = 0;
                out[count].second = 0;
                out[count].label = NULL;
                count++;
            }
        }
    unsigned cll_count = 0;
    for (size_t j = 0; j < mz_m; j++)
        if ((cllset >> j) & 1)
            cll_count++;
    cost += cll_count * MZ_C_CLL;
    if (cost >= limit)
        return false;
    mz_work += mz_natoms;
    // what is left to cover
    for (size_t a = 0; a < mz_natoms; a++)
    {
        uint16_t covered = 0;
        if ((mz_atoms[a].ccov & classes) != 0)
            covered = mz_full;
        for (size_t j = 0; j < mz_m; j++)
            if ((cllset >> j) & 1)
                covered |= mz_cmask[j];
        const uint16_t need = must[a] & (uint16_t)~covered;
        if (need == 0)
            continue;
        if (forb[a] != 0 || mz_atoms[a].group == MZ_B || mz_atoms[a].group == MZ_R ||
            (mz_atoms[a].group == MZ_F && mz_atoms[a].label == NULL))
            return false;
    }
    // intervals of numbers and runs of characters, symbols of functions
    for (unsigned pass = 0; pass < 3; pass++)
    {
        const uint8_t group = pass == 0 ? MZ_N : (pass == 1 ? MZ_O : MZ_F);
        bool open = false;
        size_t begin = 0;
        size_t end = 0;
        for (size_t a = 0; a <= mz_natoms; a++)
        {
            bool flush = false;
            if (a == mz_natoms)
                flush = open;
            else if (mz_atoms[a].group == group)
            {
                uint16_t covered = 0;
                if ((mz_atoms[a].ccov & classes) != 0)
                    covered = mz_full;
                for (size_t j = 0; j < mz_m; j++)
                    if ((cllset >> j) & 1)
                        covered |= mz_cmask[j];
                const uint16_t need = must[a] & (uint16_t)~covered;
                if (group == MZ_F)
                {
                    if (need != 0)
                    {
                        cost += MZ_C_SCF;
                        if (out != NULL)
                        {
                            if (count >= MZ_MAX_TERMS)
                                return false;
                            out[count].type = SP_SC;
                            out[count].first = 0;
                            out[count].second = 0;
                            out[count].label = mz_atoms[a].label;
                            count++;
                        }
                    }
                }
                else if (forb[a] != 0)
                    flush = open;
                else if (need != 0)
                {
                    if (!open)
                    {
                        open = true;
                        begin = a;
                    }
                    end = a;
                }
            }
            if (flush)
            {
                cost += group == MZ_N ? MZ_C_RN : MZ_C_RC;
                if (out != NULL)
                {
                    if (count >= MZ_MAX_TERMS)
                        return false;
                    out[count].type = group == MZ_N ? SP_RN : SP_RC;
                    out[count].first = mz_atoms[begin].lo;
                    out[count].second = mz_atoms[end].hi;
                    out[count].label = NULL;
                    count++;
                }
                open = false;
            }
            if (cost >= limit)
                return false;
        }
    }
    if (out != NULL)
        for (size_t j = 0; j < mz_m; j++)
            if ((cllset >> j) & 1)
            {
                if (count >= MZ_MAX_TERMS)
                    return false;
                out[count].type = SP_CLL;
                out[count].first = 0;
                out[count].second = 0;
                out[count].label = mz_rl[j];
                count++;
            }
    *cost_out = cost;
    if (nout != NULL)
        *nout = count;
    return true;
}

static bool mz_cover_best(const uint16_t *res, bool p, MZ_TERM *out, size_t *nout, uint32_t *cost_out)
{
    uint16_t must[MZ_MAX_ATOMS];
    uint16_t forb[MZ_MAX_ATOMS];
    for (size_t a = 0; a < mz_natoms; a++)
    {
        const uint16_t yes = p ? mz_atoms[a].out : (uint16_t)(~mz_atoms[a].out & mz_full);
        must[a] = res[a] & yes;
        forb[a] = res[a] & (uint16_t)(mz_full & ~yes);
    }
    unsigned cand_classes = 0;
    for (unsigned c = 0; c < 9; c++)
    {
        bool pure = true;
        bool useful = false;
        for (size_t a = 0; a < mz_natoms; a++)
            if ((mz_atoms[a].ccov >> c) & 1)
            {
                if (forb[a] != 0)
                    pure = false;
                if (must[a] != 0)
                    useful = true;
            }
        if (pure && useful)
            cand_classes |= 1u << c;
    }
    unsigned cand_cll = 0;
    for (size_t j = 0; j < mz_m; j++)
    {
        bool pure = true;
        bool useful = false;
        for (size_t a = 0; a < mz_natoms; a++)
        {
            if ((forb[a] & mz_cmask[j]) != 0)
                pure = false;
            if ((must[a] & mz_cmask[j]) != 0)
                useful = true;
        }
        if (pure && useful)
            cand_cll |= 1u << j;
    }
    uint32_t best = UINT32_MAX;
    unsigned best_classes = 0;
    unsigned best_cll = 0;
    for (unsigned i = 0; i < 512; i++)
    {
        if ((i & ~cand_classes) != 0)
            continue;
        for (unsigned j = 0; j < 16; j++)
        {
            if ((j & ~cand_cll) != 0)
                continue;
            uint32_t cost;
            if (mz_cover_eval(must, forb, i, j, NULL, NULL, &cost, best) && cost < best)
            {
                best = cost;
                best_classes = i;
                best_cll = j;
            }
            if (mz_work > MZ_WORK_LIMIT)
                return false;
        }
    }
    if (best == UINT32_MAX)
        return false;
    uint32_t cost;
    if (!mz_cover_eval(must, forb, best_classes, best_cll, out, nout, &cost, UINT32_MAX))
        return false;
    *cost_out = cost;
    return true;
}

static void mz_solve(const uint16_t *res, bool p, size_t depth, uint32_t cost)
{
    if (cost >= mz_best_cost || mz_work > MZ_WORK_LIMIT)
        return;
    {
        MZ_TERM tmp[MZ_MAX_TERMS];
        size_t count = 0;
        uint32_t c = 0;
        if (mz_cover_best(res, p, tmp, &count, &c) && cost + c < mz_best_cost && mz_cur_n + count <= MZ_MAX_TERMS)
        {
            memcpy(mz_best, mz_cur, mz_cur_n * sizeof(MZ_TERM));
            for (size_t i = 0; i < count; i++)
            {
                mz_best[mz_cur_n + i] = tmp[i];
                mz_best[mz_cur_n + i].seg = (uint8_t)depth;
            }
            mz_best_n = mz_cur_n + count;
            mz_best_segs = depth + 1;
            mz_best_cost = cost + c;
        }
    }
    if (depth + 1 >= MZ_MAX_SEGS)
        return;
    // an empty segment
    mz_solve(res, !p, depth + 1, cost + 1);
    // a segment of pure classes that resolves a part of the links
    uint16_t forb[MZ_MAX_ATOMS];
    for (size_t a = 0; a < mz_natoms; a++)
    {
        const uint16_t yes = p ? mz_atoms[a].out : (uint16_t)(~mz_atoms[a].out & mz_full);
        forb[a] = res[a] & (uint16_t)(mz_full & ~yes);
    }
    unsigned pure_classes = 0;
    for (unsigned c = 0; c < 9; c++)
    {
        bool pure = true;
        bool useful = false;
        for (size_t a = 0; a < mz_natoms; a++)
            if ((mz_atoms[a].ccov >> c) & 1)
            {
                if (forb[a] != 0)
                    pure = false;
                if (res[a] != 0)
                    useful = true;
            }
        if (pure && useful)
            pure_classes |= 1u << c;
    }
    if (pure_classes == 0)
        return;
    unsigned variants[10];
    size_t nvariants = 0;
    variants[nvariants++] = pure_classes;
    if ((pure_classes & (pure_classes - 1)) != 0)
        for (unsigned c = 0; c < 9; c++)
            if ((pure_classes >> c) & 1)
                variants[nvariants++] = 1u << c;
    for (size_t v = 0; v < nvariants; v++)
    {
        const unsigned set = variants[v];
        uint16_t next[MZ_MAX_ATOMS];
        bool changed = false;
        for (size_t a = 0; a < mz_natoms; a++)
        {
            next[a] = res[a];
            if ((mz_atoms[a].ccov & set) != 0 && res[a] != 0)
            {
                next[a] = 0;
                changed = true;
            }
        }
        if (!changed)
            continue;
        const size_t save = mz_cur_n;
        uint32_t added = 0;
        bool overflow = false;
        for (unsigned c = 0; c < 9; c++)
            if ((set >> c) & 1)
            {
                if (mz_cur_n >= MZ_MAX_TERMS)
                {
                    overflow = true;
                    break;
                }
                mz_cur[mz_cur_n].type = mz_class_code[c];
                mz_cur[mz_cur_n].first = 0;
                mz_cur[mz_cur_n].second = 0;
                mz_cur[mz_cur_n].label = NULL;
                mz_cur[mz_cur_n].seg = (uint8_t)depth;
                mz_cur_n++;
                added++;
            }
        if (!overflow)
            mz_solve(next, !p, depth + 1, cost + added + 1);
        mz_cur_n = save;
    }
}

// returns false if the specifier matches any link (no check is needed)
static bool minimize_buffer(void)
{
    if (spec_buffer_size == 0)
        return true;
    T_SP_TERM *terms = malloc(spec_buffer_size * sizeof(T_SP_TERM));
    if (terms == NULL)
        error_no_memory();
    size_t n = 0;
    size_t segs = 0;
    if (!parse_terms(spec_buffer, spec_buffer_size, pending_addresses, pending_count, terms, &n, &segs))
    {
        free(terms);
        return true;
    }
    T_LABEL *cl[MZ_MAX_CLL];
    size_t ncl = 0;
    T_LABEL *lf[MZ_MAX_LABELS];
    size_t nlf = 0;
    uint32_t *bp = malloc((2 * n + 2) * sizeof(uint32_t));
    if (bp == NULL)
        error_no_memory();
    size_t nbp = 0;
    bp[nbp++] = 0;
    bool ok = true;
    for (size_t i = 0; i < n && ok; i++)
    {
        const uint8_t *pl = spec_buffer + terms[i].offset;
        switch (terms[i].type)
        {
        case SP_CLL:
            ok = mz_insert_label(cl, &ncl, MZ_MAX_CLL, terms[i].label);
            break;
        case SP_SC:
        {
            uint32_t tag;
            memcpy(&tag, pl, ZBLL);
            if (tag == TAGF)
                ok = mz_insert_label(lf, &nlf, MZ_MAX_LABELS, terms[i].label);
            else if (tag == TAGN)
            {
                uint32_t number;
                memcpy(&number, pl + ZBLL, ZBLL);
                bp[nbp++] = number;
                if (number < UINT32_MAX)
                    bp[nbp++] = number + 1;
            }
            else if (tag != TAGO)
                ok = false;
            break;
        }
        case SP_RN:
        {
            uint32_t first;
            uint32_t second;
            memcpy(&first, pl, ZBLL);
            memcpy(&second, pl + ZBLL, ZBLL);
            bp[nbp++] = first;
            if (second < UINT32_MAX)
                bp[nbp++] = second + 1;
            break;
        }
        default:
            break;
        }
    }
    if (!ok)
    {
        free(bp);
        free(terms);
        return true;
    }
    qsort(bp, nbp, sizeof(uint32_t), mz_u32_compare);
    size_t nu = 0;
    for (size_t i = 0; i < nbp; i++)
        if (nu == 0 || bp[nu - 1] != bp[i])
            bp[nu++] = bp[i];
    nbp = nu;
    // raw atoms: brackets, references, other functions, functions of symbols, intervals, characters
    const size_t raw_max = 3 + nlf + nbp + 256;
    MZ_ATOM *raw = malloc(raw_max * sizeof(MZ_ATOM));
    MZ_PROBE *raw_probe = malloc(raw_max * sizeof(MZ_PROBE));
    uint32_t *raw_out = malloc(raw_max * sizeof(uint32_t));
    if (raw == NULL || raw_probe == NULL || raw_out == NULL)
        error_no_memory();
    size_t nraw = 0;
#define MZ_RAW(g, l, h, lab, tg, vl)            \
    do                                          \
    {                                           \
        memset(&raw[nraw], 0, sizeof(MZ_ATOM)); \
        raw[nraw].group = (g);                  \
        raw[nraw].lo = (l);                     \
        raw[nraw].hi = (h);                     \
        raw[nraw].label = (lab);                \
        raw_probe[nraw].tag = (tg);             \
        raw_probe[nraw].val = (vl);             \
        raw_probe[nraw].label = (lab);          \
        nraw++;                                 \
    } while (0)
    MZ_RAW(MZ_B, 0, 0, NULL, 1, 0);
    MZ_RAW(MZ_R, 0, 0, NULL, TAGR, 0);
    MZ_RAW(MZ_F, 0, 0, NULL, TAGF, 0);
    for (size_t i = 0; i < nlf; i++)
        MZ_RAW(MZ_F, 0, 0, lf[i], TAGF, 0);
    for (size_t i = 0; i < nbp; i++)
        MZ_RAW(MZ_N, bp[i], i + 1 < nbp ? bp[i + 1] - 1 : UINT32_MAX, NULL, TAGN, bp[i]);
    for (uint32_t c = 0; c < 256; c++)
    {
        MZ_RAW(MZ_O, c, c, NULL, TAGO, c);
        raw[nraw - 1].digit = isdigit((int)c) != 0;
        raw[nraw - 1].alpha = isalpha((int)c) != 0;
    }
#undef MZ_RAW
    const uint32_t ncv = 1u << ncl;
    for (size_t i = 0; i < nraw; i++)
    {
        raw_out[i] = 0;
        for (uint32_t c = 0; c < ncv; c++)
            if (mz_eval(spec_buffer, terms, n, segs, &raw_probe[i], c, cl, ncl))
                raw_out[i] |= 1u << c;
    }
    // black boxes that influence the answer
    size_t relevant[MZ_MAX_CLL];
    size_t nrel = 0;
    for (size_t j = 0; j < ncl; j++)
    {
        bool rel = false;
        for (size_t i = 0; i < nraw && !rel; i++)
            for (uint32_t c = 0; c < ncv && !rel; c++)
                if (((c >> j) & 1) == 0 && ((raw_out[i] >> c) & 1) != ((raw_out[i] >> (c | (1u << j))) & 1))
                    rel = true;
        if (rel)
        {
            relevant[nrel] = j;
            mz_rl[nrel] = cl[j];
            nrel++;
        }
    }
    mz_m = nrel;
    const uint32_t nc = 1u << mz_m;
    mz_full = nc == 16 ? (uint16_t)0xFFFF : (uint16_t)((1u << nc) - 1);
    for (size_t k = 0; k < mz_m; k++)
    {
        mz_cmask[k] = 0;
        for (uint32_t r = 0; r < nc; r++)
            if ((r >> k) & 1)
                mz_cmask[k] |= (uint16_t)(1u << r);
    }
    for (size_t i = 0; i < nraw; i++)
    {
        uint16_t reduced = 0;
        for (uint32_t r = 0; r < nc; r++)
        {
            uint32_t c = 0;
            for (size_t k = 0; k < mz_m; k++)
                if ((r >> k) & 1)
                    c |= 1u << relevant[k];
            if ((raw_out[i] >> c) & 1)
                reduced |= (uint16_t)(1u << r);
        }
        raw[i].out = reduced;
    }
    // final atoms: equal neighbours are merged
    mz_natoms = 0;
    bool too_many = false;
    uint16_t other_f = 0;
    for (size_t i = 0; i < nraw && !too_many; i++)
    {
        const MZ_ATOM *r = &raw[i];
        if (r->group == MZ_F && r->label == NULL)
            other_f = r->out;
    }
    for (size_t i = 0; i < nraw && !too_many; i++)
    {
        const MZ_ATOM *r = &raw[i];
        if (r->group == MZ_F && r->label != NULL && r->out == other_f)
            continue;
        if (mz_natoms > 0)
        {
            MZ_ATOM *last = &mz_atoms[mz_natoms - 1];
            if (r->group == last->group && r->out == last->out &&
                ((r->group == MZ_N) || (r->group == MZ_O && r->digit == last->digit && r->alpha == last->alpha)))
            {
                last->hi = r->hi;
                continue;
            }
        }
        if (mz_natoms >= MZ_MAX_ATOMS)
        {
            too_many = true;
            break;
        }
        mz_atoms[mz_natoms] = *r;
        mz_natoms++;
    }
    free(raw);
    free(raw_probe);
    free(raw_out);
    if (too_many)
    {
        free(bp);
        free(terms);
        return true;
    }
    bool all_yes = true;
    for (size_t a = 0; a < mz_natoms; a++)
    {
        if (mz_atoms[a].out != mz_full)
            all_yes = false;
        mz_atoms[a].ccov = 0;
        for (unsigned c = 0; c < 9; c++)
            if (mz_class_covers(c, &mz_atoms[a]))
                mz_atoms[a].ccov |= (uint16_t)(1u << c);
    }
    if (all_yes)
    {
        free(bp);
        free(terms);
        return false;
    }
    // search
    uint16_t res[MZ_MAX_ATOMS];
    for (size_t a = 0; a < mz_natoms; a++)
        res[a] = mz_full;
    mz_best_cost = UINT32_MAX;
    mz_best_n = 0;
    mz_best_segs = 0;
    mz_cur_n = 0;
    mz_work = 0;
    mz_solve(res, true, 0, 1);
    if (mz_best_cost == UINT32_MAX || mz_best_cost > spec_buffer_size)
    {
        free(bp);
        free(terms);
        return true;
    }
    // encode the result
    uint8_t *nb = malloc(mz_best_cost + 16);
    T_PENDING_ADDRESS *np = malloc((mz_best_n + 1) * sizeof(T_PENDING_ADDRESS));
    if (nb == NULL || np == NULL)
        error_no_memory();
    size_t nsize = 0;
    size_t npc = 0;
    bool encoded = true;
    for (size_t s = 0; s < mz_best_segs && encoded; s++)
    {
        if (s > 0)
            nb[nsize++] = SP_NG;
        for (size_t i = 0; i < mz_best_n && encoded; i++)
        {
            if (mz_best[i].seg != s)
                continue;
            nb[nsize++] = mz_best[i].type;
            switch (mz_best[i].type)
            {
            case SP_RN:
                memcpy(nb + nsize, &mz_best[i].first, ZBLL);
                memcpy(nb + nsize + ZBLL, &mz_best[i].second, ZBLL);
                nsize += 2 * ZBLL;
                break;
            case SP_RC:
                nb[nsize++] = (uint8_t)mz_best[i].first;
                nb[nsize++] = (uint8_t)mz_best[i].second;
                break;
            case SP_SC:
            {
                const uint32_t tag = TAGF;
                memcpy(nb + nsize, &tag, ZBLL);
                nsize += ZBLL;
                memset(nb + nsize, 0, LBLL);
                np[npc].offset = nsize;
                np[npc].label = mz_best[i].label;
                npc++;
                nsize += LBLL;
                break;
            }
            case SP_CLL:
                memset(nb + nsize, 0, LBLL);
                np[npc].offset = nsize;
                np[npc].label = mz_best[i].label;
                npc++;
                nsize += LBLL;
                break;
            default:
                break;
            }
            if (nsize > mz_best_cost)
                encoded = false;
        }
    }
    if (encoded)
        nb[nsize++] = SP_NGW;
    if (!encoded || nsize != mz_best_cost)
    {
        free(nb);
        free(np);
        free(bp);
        free(terms);
        return true;
    }
    // verification on all kinds of links and all values of black boxes
    T_SP_TERM *nterms = malloc(nsize * sizeof(T_SP_TERM));
    if (nterms == NULL)
        error_no_memory();
    size_t nn = 0;
    size_t nsegs = 0;
    bool same = parse_terms(nb, nsize, np, npc, nterms, &nn, &nsegs);
    for (size_t i = 0; i < nn && same; i++)
        if (nterms[i].type == SP_CLL)
        {
            bool known = false;
            for (size_t j = 0; j < ncl; j++)
                if (cl[j] == nterms[i].label)
                    known = true;
            if (!known)
                same = false;
        }
    if (same)
    {
        MZ_PROBE probe;
        probe.label = NULL;
        for (uint32_t c = 0; c < ncv && same; c++)
        {
            for (uint32_t tag = 0; tag < 8 && same; tag++)
            {
                if (tag == TAGO || tag == TAGN || tag == TAGF)
                    continue;
                probe.tag = tag;
                probe.val = 0;
                probe.label = NULL;
                if (mz_eval(spec_buffer, terms, n, segs, &probe, c, cl, ncl) != mz_eval(nb, nterms, nn, nsegs, &probe, c, cl, ncl))
                    same = false;
            }
            probe.tag = TAGO;
            probe.label = NULL;
            for (uint32_t v = 0; v < 256 && same; v++)
            {
                probe.val = v;
                if (mz_eval(spec_buffer, terms, n, segs, &probe, c, cl, ncl) != mz_eval(nb, nterms, nn, nsegs, &probe, c, cl, ncl))
                    same = false;
            }
            probe.tag = TAGN;
            for (size_t i = 0; i < nbp + 3 && same; i++)
            {
                uint32_t v;
                if (i == 0)
                    v = UINT32_MAX;
                else if (i == 1)
                    v = 1;
                else if (i == 2)
                    v = UINT32_MAX - 1;
                else
                    v = bp[i - 3];
                for (int d = -1; d <= 1 && same; d++)
                {
                    if ((d < 0 && v == 0) || (d > 0 && v == UINT32_MAX))
                        continue;
                    probe.val = v + (uint32_t)d;
                    if (mz_eval(spec_buffer, terms, n, segs, &probe, c, cl, ncl) != mz_eval(nb, nterms, nn, nsegs, &probe, c, cl, ncl))
                        same = false;
                }
            }
            probe.tag = TAGF;
            probe.val = 0;
            for (size_t i = 0; i <= nlf && same; i++)
            {
                probe.label = i < nlf ? lf[i] : NULL;
                if (mz_eval(spec_buffer, terms, n, segs, &probe, c, cl, ncl) != mz_eval(nb, nterms, nn, nsegs, &probe, c, cl, ncl))
                    same = false;
            }
        }
    }
    if (same)
    {
        memcpy(spec_buffer, nb, nsize);
        spec_buffer_size = nsize;
        if (npc > 0)
            memcpy(pending_addresses, np, npc * sizeof(T_PENDING_ADDRESS));
        pending_count = npc;
    }
    free(nterms);
    free(nb);
    free(np);
    free(bp);
    free(terms);
    return true;
}

T_LABEL *specifier_pool_find_or_create(void)
{
    collecting = false;
    if (!canonicalize_buffer())
        return NULL;
    if (!minimize_buffer())
        return NULL;
#if defined mdebug
    fprintf(stderr, "specifier_pool_find_or_create: searching, buffer_size=%zu\n", spec_buffer_size);
    fprintf(stderr, "  buffer content: ");
    for (size_t i = 0; i < spec_buffer_size && i < 48; i++)
        fprintf(stderr, "%02X ", spec_buffer[i]);
    if (spec_buffer_size > 48)
        fprintf(stderr, "...");
    fprintf(stderr, "\n");
#endif
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        if (current->length != spec_buffer_size)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: length mismatch (%zu != %zu)\n", (void *)current, current->length, spec_buffer_size);
#endif
            current = current->next;
            continue;
        }
        if (current->address_count != pending_count)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: address_count mismatch (%zu != %zu)\n", (void *)current, current->address_count, pending_count);
#endif
            current = current->next;
            continue;
        }
        if (memcmp(current->bytes, spec_buffer, spec_buffer_size) != 0)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: bytes mismatch\n", (void *)current);
#endif
            current = current->next;
            continue;
        }
        bool addresses_match = true;
        for (size_t i = 0; i < pending_count; i++)
        {
            if (current->addresses[i].offset != pending_addresses[i].offset)
            {
                addresses_match = false;
                break;
            }
            if (current->addresses[i].label != pending_addresses[i].label)
            {
                addresses_match = false;
                break;
            }
        }
        if (addresses_match)
        {
#if defined mdebug
            fprintf(stderr, "  FOUND existing label=%p\n", (void *)current->label);
#endif
            return current->label;
        }
#if defined mdebug
        fprintf(stderr, "  skip spec at %p: addresses mismatch\n", (void *)current);
#endif
        current = current->next;
    }
    T_UNIQUE_SPECIFIER *new_spec = malloc(sizeof(T_UNIQUE_SPECIFIER));
    if (new_spec == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec=%p\n", (void *)new_spec);
#endif
    new_spec->bytes = malloc(spec_buffer_size);
    if (new_spec->bytes == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec->bytes=%p length=%zu\n", (void *)new_spec->bytes, spec_buffer_size);
#endif
    memcpy(new_spec->bytes, spec_buffer, spec_buffer_size);
    new_spec->length = spec_buffer_size;
    new_spec->label = (T_LABEL *)generate_info_label();
    new_spec->address_count = pending_count;
    if (pending_count > 0)
    {
        new_spec->addresses = malloc(pending_count * sizeof(T_PENDING_ADDRESS));
        if (new_spec->addresses == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec->addresses=%p count=%zu\n", (void *)new_spec->addresses, pending_count);
#endif
        memcpy(new_spec->addresses, pending_addresses, pending_count * sizeof(T_PENDING_ADDRESS));
    }
    else
        new_spec->addresses = NULL;
    new_spec->next = pool_head;
    pool_head = new_spec;
#if defined mdebug
    fprintf(stderr, "  CREATED new label=%p\n", (void *)new_spec->label);
#endif
    return new_spec->label;
}

static bool specifiers_equal(const T_UNIQUE_SPECIFIER *a, const T_UNIQUE_SPECIFIER *b)
{
    if (a->length != b->length)
        return false;
    if (a->address_count != b->address_count)
        return false;
    if (memcmp(a->bytes, b->bytes, a->length) != 0)
        return false;
    for (size_t i = 0; i < a->address_count; i++)
    {
        if (a->addresses[i].offset != b->addresses[i].offset)
            return false;
        if (a->addresses[i].label != b->addresses[i].label)
            return false;
    }
    return true;
}

void specifier_pool_finalize(void)
{
#if defined mdebug
    fprintf(stderr, "specifier_pool_finalize: begin\n");
#endif
    // === PASS 1 ===
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        for (size_t i = 0; i < current->address_count; i++)
            current->addresses[i].label = resolve_label_alias(current->addresses[i].label);
        current = current->next;
    }
    // === PASS 2 ===
    current = pool_head;
    while (current != NULL)
    {
        T_UNIQUE_SPECIFIER **prev_next = &current->next;
        T_UNIQUE_SPECIFIER *other = current->next;
        while (other != NULL)
        {
            if (specifiers_equal(current, other))
            {
#if defined mdebug
                fprintf(stderr, "  MERGE: label=%p -> label=%p\n", (void *)other->label, (void *)current->label);
#endif
                macrocode_equ((T_LABEL *)other->label, (T_LABEL *)current->label);
                *prev_next = other->next;
                free(other->bytes);
                if (other->addresses != NULL)
                    free(other->addresses);
                free(other);
                other = *prev_next;
            }
            else
            {
                prev_next = &other->next;
                other = other->next;
            }
        }
        current = current->next;
    }
    // === PASS 3 ===
    current = pool_head;
    while (current != NULL)
    {
        macrocode_label(current->label);
#if defined mdebug
        fprintf(stderr, "  WRITE: label=%p length=%zu\n", (void *)current->label, current->length);
#endif
        size_t pos = 0;
        size_t addr_idx = 0;
        size_t next_label_offset = (addr_idx < current->address_count) ? current->addresses[addr_idx].offset : current->length;
        while (pos < current->length)
        {
            if (pos == next_label_offset)
            {
                macrocode_address(current->addresses[addr_idx].label);
                pos += LBLL;
                addr_idx++;
                next_label_offset = (addr_idx < current->address_count) ? current->addresses[addr_idx].offset : current->length;
            }
            else
            {
                macrocode_byte(current->bytes[pos]);
                pos++;
            }
        }
        current = current->next;
    }
#if defined mdebug
    fprintf(stderr, "specifier_pool_finalize: end\n");
#endif
}

//----------  end of file specifier_pool.c  ----------
