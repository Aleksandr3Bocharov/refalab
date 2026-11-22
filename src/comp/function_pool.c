// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-21
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file function_pool.c  ----------
//    pool of functions for prefix optimization
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "function_pool.h"
#include "avl_identifiers.h"
#include "compile_output.h"
#include "compile_sentence.h"
#include "print_errors.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "compiler.h"

// The list of stored functions of the module and the function being compiled.
static T_STORED_FUNCTION *function_pool_head = NULL;
static T_STORED_FUNCTION *function_pool_tail = NULL;
static T_STORED_FUNCTION *current_function = NULL;

// Starts an empty pool.
void function_pool_init(void)
{
    function_pool_head = NULL;
    function_pool_tail = NULL;
    current_function = NULL;
    return;
}

// Releases the memory of a buffer and makes it empty.
static void clear_operator_buffer(T_OPERATOR_BUFFER *buf)
{
    if (buf->bytes != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(clear_operator_buffer): bytes=%p\n", (void *)buf->bytes);
#endif
        free(buf->bytes);
    }
    if (buf->labels != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(clear_operator_buffer): labels=%p\n", (void *)buf->labels);
#endif
        free(buf->labels);
    }
    if (buf->statements != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(clear_operator_buffer): statements=%p\n", (void *)buf->statements);
#endif
        free(buf->statements);
    }
    buf->bytes = NULL;
    buf->length = 0;
    buf->capacity = 0;
    buf->labels = NULL;
    buf->label_count = 0;
    buf->label_capacity = 0;
    buf->statements = NULL;
    buf->statement_count = 0;
    buf->statement_capacity = 0;
    return;
}

// Releases all functions with their sentences.
void function_pool_clear(void)
{
    T_STORED_FUNCTION *func = function_pool_head;
    while (func != NULL)
    {
        T_STORED_FUNCTION *next_func = func->next;
        if (func->func_name != NULL)
        {
#if defined mdebug
            fprintf(stderr, "free(function_pool_clear): func_name=%p\n", (void *)func->func_name);
#endif
            free(func->func_name);
        }
        T_STORED_SENTENCE *sentence = func->sentences;
        while (sentence != NULL)
        {
            T_STORED_SENTENCE *next_sentence = sentence->next;
            clear_operator_buffer(&sentence->left_part);
            clear_operator_buffer(&sentence->right_part);
#if defined mdebug
            fprintf(stderr, "free(function_pool_clear): sentence=%p\n", (void *)sentence);
#endif
            free(sentence);
            sentence = next_sentence;
        }
#if defined mdebug
        fprintf(stderr, "free(function_pool_clear): func=%p\n", (void *)func);
#endif
        free(func);
        func = next_func;
    }
    function_pool_head = NULL;
    function_pool_tail = NULL;
    return;
}

// The function being compiled (NULL after an error in its definition).
T_STORED_FUNCTION *function_pool_get_current_function(void)
{
    return current_function;
}

void function_pool_set_current_function(T_STORED_FUNCTION *func)
{
    current_function = func;
    return;
}

// Appends a new empty function with the given label to the end of the pool.
T_STORED_FUNCTION *function_pool_begin(T_LABEL *label)
{
    T_STORED_FUNCTION *func = (T_STORED_FUNCTION *)calloc(1, sizeof(T_STORED_FUNCTION));
    if (func == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(function_pool_begin): func=%p label=%p\n", (void *)func, (void *)label);
#endif
    func->func_label = label;
    func->func_name = NULL;
    func->func_name_length = 0;
    func->sentences = NULL;
    func->last_sentence = NULL;
    func->sentence_count = 0;
    func->next = NULL;
    if (function_pool_tail != NULL)
        function_pool_tail->next = func;
    else
        function_pool_head = func;
    function_pool_tail = func;
    return func;
}

// Stores the name that is written before the function code. The name is kept only
// when the names option is on; with the full-name option it is prefixed with the
// module name and ':'. A name longer than 255 characters is truncated in its length byte.
void function_pool_set_name(T_STORED_FUNCTION *func, const char *name, uint8_t name_length)
{
    if (func->func_name != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(function_pool_set_name): func_name=%p\n", (void *)func->func_name);
#endif
        free(func->func_name);
        func->func_name = NULL;
    }
    if (!options.names)
    {
        func->func_name = NULL;
        func->func_name_length = 0;
        return;
    }
    size_t total_length;
    char *full_name;
    if (options.full_name)
    {
        // module_name + ':' + identifier
        total_length = (size_t)scanner.module_name_length + 1 + name_length;
        full_name = (char *)malloc(total_length);
        if (full_name == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(function_pool_set_name): full_name=%p length=%zu (full)\n", (void *)full_name, total_length);
#endif
        memcpy(full_name, scanner.module_name, scanner.module_name_length);
        full_name[scanner.module_name_length] = ':';
        memcpy(full_name + scanner.module_name_length + 1, name, name_length);
    }
    else
    {
        // only identifier
        total_length = name_length;
        full_name = (char *)malloc(total_length);
        if (full_name == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(function_pool_set_name): full_name=%p length=%zu\n", (void *)full_name, total_length);
#endif
        memcpy(full_name, name, name_length);
    }
    func->func_name = full_name;
    func->func_name_length = (uint8_t)(total_length > UINT8_MAX ? UINT8_MAX : total_length);
    return;
}

// Appends an empty sentence to the function; returns NULL when there is no function.
T_STORED_SENTENCE *function_pool_add_sentence(T_STORED_FUNCTION *func, T_LABEL *sentence_label)
{
    if (func == NULL)
        return NULL;
    T_STORED_SENTENCE *sentence = (T_STORED_SENTENCE *)calloc(1, sizeof(T_STORED_SENTENCE));
    if (sentence == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(function_pool_add_sentence): sentence=%p label=%p\n", (void *)sentence, (void *)sentence_label);
#endif
    sentence->sentence_label = sentence_label;
    sentence->left_part.bytes = NULL;
    sentence->left_part.length = 0;
    sentence->left_part.capacity = 0;
    sentence->left_part.labels = NULL;
    sentence->left_part.label_count = 0;
    sentence->left_part.label_capacity = 0;
    sentence->left_part.statements = NULL;
    sentence->left_part.statement_count = 0;
    sentence->left_part.statement_capacity = 0;
    sentence->right_part.bytes = NULL;
    sentence->right_part.length = 0;
    sentence->right_part.capacity = 0;
    sentence->right_part.labels = NULL;
    sentence->right_part.label_count = 0;
    sentence->right_part.label_capacity = 0;
    sentence->right_part.statements = NULL;
    sentence->right_part.statement_count = 0;
    sentence->right_part.statement_capacity = 0;
    sentence->next = NULL;
    if (func->last_sentence != NULL)
        func->last_sentence->next = sentence;
    else
        func->sentences = sentence;
    func->last_sentence = sentence;
    func->sentence_count++;
    return sentence;
}

// Selects the destination of the generated code.
void function_pool_set_output_mode(T_STORED_SENTENCE *sentence, bool is_left_part)
{
    if (sentence == NULL)
        compile_output_set_mode(OUTPUT_MACROCODE, NULL);
    else
    {
        const T_OUTPUT_MODE mode = is_left_part ? OUTPUT_LEFT_PART : OUTPUT_RIGHT_PART;
        compile_output_set_mode(mode, sentence);
    }
    return;
}

// Makes the buffer able to hold at least `needed` elements: starts from
// `initial_capacity` and doubles the size when necessary.
static void *ensure_buffer_capacity(void *buffer, size_t *capacity, size_t needed, size_t element_size, size_t initial_capacity, const char *debug_name)
{
    if (*capacity >= needed)
        return buffer;
    size_t new_capacity = (*capacity == 0) ? initial_capacity : *capacity * 2;
    while (new_capacity < needed)
        new_capacity *= 2;  
    void *new_buffer = realloc(buffer, new_capacity * element_size);
    if (new_buffer == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "realloc(%s): buffer=%p capacity=%zu\n", debug_name, new_buffer, new_capacity);
#endif
    *capacity = new_capacity;
    return new_buffer;
}

// Capacity of the byte area: initially 64 bytes.
static inline void ensure_capacity(T_OPERATOR_BUFFER *buf, size_t needed)
{
    buf->bytes = ensure_buffer_capacity(buf->bytes, &buf->capacity, needed, sizeof(uint8_t), 64, "ensure_capacity");
    return;
}

// Capacity of the label list: initially 8 labels.
static inline void ensure_label_capacity(T_OPERATOR_BUFFER *buf, size_t needed)
{
    buf->labels = ensure_buffer_capacity(buf->labels, &buf->label_capacity, needed, sizeof(T_LABEL_REF), 8, "ensure_label_capacity");
    return;
}

// Capacity of the list of statement offsets: initially 8 statements.
static inline void ensure_statement_capacity(T_OPERATOR_BUFFER *buf, size_t needed)
{
    buf->statements = ensure_buffer_capacity(buf->statements, &buf->statement_capacity, needed, sizeof(size_t), 8, "ensure_statement_capacity");
    return;
}

// Remembers the offset of a new statement of the left part: it starts at the current end of the buffer.
void sentence_mark_statement_left(T_STORED_SENTENCE *sentence)
{
    T_OPERATOR_BUFFER *buf = &sentence->left_part;
    ensure_statement_capacity(buf, buf->statement_count + 1);
    buf->statements[buf->statement_count++] = buf->length;
    return;
}

// Appending a byte.
void sentence_append_byte_left(T_STORED_SENTENCE *sentence, uint8_t byte)
{
    T_OPERATOR_BUFFER *buf = &sentence->left_part;
    ensure_capacity(buf, buf->length + 1);
    buf->bytes[buf->length++] = byte;
    return;
}

void sentence_append_byte_right(T_STORED_SENTENCE *sentence, uint8_t byte)
{
    T_OPERATOR_BUFFER *buf = &sentence->right_part;
    ensure_capacity(buf, buf->length + 1);
    buf->bytes[buf->length++] = byte;
    return;
}

// Appending an address: a zeroed slot of LBLL bytes is reserved and the label
// is remembered with the offset of the slot; the slot is filled when the buffer is written.
void sentence_append_address_left(T_STORED_SENTENCE *sentence, T_LABEL *label)
{
    T_OPERATOR_BUFFER *buf = &sentence->left_part;
    ensure_capacity(buf, buf->length + LBLL);
    ensure_label_capacity(buf, buf->label_count + 1);
    buf->labels[buf->label_count].offset = buf->length;
    buf->labels[buf->label_count].label = label;
    buf->label_count++;
    memset(buf->bytes + buf->length, 0, LBLL);
    buf->length += LBLL;
    return;
}

void sentence_append_address_right(T_STORED_SENTENCE *sentence, T_LABEL *label)
{
    T_OPERATOR_BUFFER *buf = &sentence->right_part;
    ensure_capacity(buf, buf->length + LBLL);
    ensure_label_capacity(buf, buf->label_count + 1);
    buf->labels[buf->label_count].offset = buf->length;
    buf->labels[buf->label_count].label = label;
    buf->label_count++;
    memset(buf->bytes + buf->length, 0, LBLL);
    buf->length += LBLL;
    return;
}

// Copies the range [from, to) of the buffer to the macrocode, replacing every reserved
// address slot by the address of its label. label_index is the index in labels[] of the first
// address slot that is not before `from`.
static void write_range_to_macrocode(const T_OPERATOR_BUFFER *buf, size_t from, size_t to, size_t label_index)
{
    size_t pos = from;
    size_t label_idx = label_index;
    size_t next_label_offset = (label_idx < buf->label_count) ? buf->labels[label_idx].offset : to;
    while (pos < to)
    {
        if (pos == next_label_offset)
        {
            macrocode_address(buf->labels[label_idx].label);
            pos += LBLL;
            label_idx++;
            next_label_offset = (label_idx < buf->label_count) ? buf->labels[label_idx].offset : to;
        }
        else
        {
            macrocode_byte(buf->bytes[pos]);
            pos++;
        }
    }
    return;
}

// Copies the whole buffer to the macrocode.
static inline void write_buffer_to_macrocode(const T_OPERATOR_BUFFER *buf)
{
    write_range_to_macrocode(buf, 0, buf->length, 0);
    return;
}

// Writes the name of a function and its length byte (or a zero byte without a name).
static void write_function_name(T_STORED_FUNCTION *func)
{
    if (func->func_name != NULL)
    {
        for (uint8_t i = 0; i < func->func_name_length; i++)
            macrocode_byte((uint8_t)func->func_name[i]);
        macrocode_byte(func->func_name_length);
    }
    else
        macrocode_byte(0);
    return;
}

//--------------------------------------------------------------------------
//  Prefix optimization: merging of the common beginnings of the left parts
//--------------------------------------------------------------------------
// The sentences of a function are tried one after another, so when the left parts of
// neighbouring sentences begin with the same statements, these statements are executed
// again and again for nothing. The statement n_sjump puts a point to return to on the
// jump stack, so the common statements can be written once and the sentences can
// branch after them (see S.A. Romanenko, "Realization of Refal-2", 1987, sections 3.11 and 3.12):
//
//     F: n_sjump(L1); A; B; C1; ...  right part 1
//     L1: A; B; C2; ...  right part 2          =>     F: A; B; n_sjump(L1); C1; ...  right part 1
//                                                     L1: C2; ...  right part 2
//
// The left part of a sentence is cut into terms, which are the arcs of a tree of the function:
//   - a term is a single statement that makes one deterministic check (a symbol, a bracket,
//     a variable bound earlier, a type specifier and so on);
//   - a term is also a whole group of statements that opens a search for an e-variable (n_ple, n_plv,
//     n_pleb, ... together with the statement that expands the variable) and goes up to the
//     n_eoe/n_eoei statement that removes all the points of the group from the jump stack. Such a
//     group finds the first suitable value of the variables and does not return to
//     it afterwards, so for the rest of the left part it behaves like a single deterministic
//     statement ("a composite statement", section 3.12). A search that is not closed by
//     n_eoe/n_eoei cannot be merged, because the statements after it may return to it.
// The rest of the left part (from the first thing that is not a term up to n_eor) and the right part
// are never merged.
// Only the neighbouring sentences are merged, otherwise the order of the sentences would change.
// A node of the tree with several children is written as
//
//             n_sjump(L2); child 1
//     L2:     n_sjump(L3); child 2
//     ...
//     Lk:     child k
//
// where the label before the child that begins with the sentence j is the label of the
// sentence j-1 (the label that follows that sentence in the non-optimized code).
// The last child has no n_sjump: when it fails, the point left by an enclosing node (or an empty
// jump stack, which means that the recognition is impossible) is used.

// The size of the jump stack of the interpreter (src/inter/interpreter.c), and the limit
// of the points that the nodes of the tree may put on it along one path.
#define JUMP_STACK_SIZE 128
#define MAX_MERGE_LEVELS 32

// A term of a left part: the range of its bytes in the left part buffer and the range of its address slots.
typedef struct term
{
    size_t offset;
    size_t end;
    size_t label_first;
    size_t label_end;
} T_TERM;

// The left part of a sentence cut into terms.
typedef struct sentence_terms
{
    T_TERM *items;           // the terms from the beginning of the left part
    size_t item_count;
    size_t tail_offset;      // the offset of the rest of the left part that is not merged (it ends with n_eor)
    size_t tail_label_first; // the index of the first address slot of the rest
    size_t max_depth;        // the maximal number of points that the left part puts on the jump stack
} T_SENTENCE_TERMS;

// The sentences of a function and the terms of their left parts.
typedef struct merge_context
{
    T_STORED_SENTENCE **sentences;
    T_SENTENCE_TERMS *terms;
    size_t max_levels; // the limit of the n_sjump points of the tree along one path
} T_MERGE_CONTEXT;

// The statements that begin a search for an e-variable (or for a v-variable), the first
// statement of an "open" pair: it is always followed by the statement that expands the variable.
static bool is_open_statement(uint8_t operator)
{
    switch (operator)
    {
    case n_ple:
    case n_plv:
    case n_pre:
    case n_prv:
    case n_plesc:
    case n_plvsc:
    case n_presc:
    case n_prvsc:
    case n_pleb:
    case n_plvb:
    case n_preb:
    case n_prvb:
    case n_plespc:
    case n_prespc:
        return true;
    default:
        return false;
    }
}

// The statements that expand the variable of a search; the interpreter returns to them after a failure.
static bool is_continue_statement(uint8_t operator)
{
    switch (operator)
    {
    case n_le:
    case n_re:
    case n_lesc:
    case n_resc:
    case n_lesd:
    case n_resd:
    case n_leb:
    case n_reb:
    case n_lespc:
    case n_respc:
        return true;
    default:
        return false;
    }
}

// The statements that make one deterministic check and never put a point on the jump stack
// nor remove one (n_lsrch and n_rsrch scan the expression inside the statement).
static bool is_simple_statement(uint8_t operator)
{
    switch (operator)
    {
    case n_sb1b2:
    case n_lsc:
    case n_rsc:
    case n_lsco:
    case n_rsco:
    case n_lsd:
    case n_rsd:
    case n_ltxt:
    case n_rtxt:
    case n_lb:
    case n_lby:
    case n_rb:
    case n_rby:
    case n_ls:
    case n_rs:
    case n_lw:
    case n_rw:
    case n_lbnil:
    case n_rbnil:
    case n_lbce:
    case n_rbce:
    case n_nil:
    case n_ce:
    case n_led:
    case n_red:
    case n_nnil:
    case n_lsrch:
    case n_rsrch:
    case n_wspc:
    case n_espc:
    case n_lmax:
    case n_rmax:
        return true;
    default:
        return false;
    }
}

// The operator code of the statement with the given index.
static inline uint8_t statement_code(const T_OPERATOR_BUFFER *buf, size_t index)
{
    return buf->bytes[buf->statements[index]];
}

// The number of points that n_eoe/n_eoei removes from the jump stack (the operand of n_eoe
// follows its operator code).
static inline size_t removed_points(const T_OPERATOR_BUFFER *buf, size_t index)
{
    return (statement_code(buf, index) == n_eoe) ? buf->bytes[buf->statements[index] + sizeof(uint8_t)] : 1;
}

// The index of the first address slot of the buffer that is not before the offset.
static size_t first_label_at(const T_OPERATOR_BUFFER *buf, size_t offset)
{
    size_t low = 0;
    size_t high = buf->label_count;
    while (low < high)
    {
        const size_t middle = low + (high - low) / 2;
        if (buf->labels[middle].offset < offset)
            low = middle + 1;
        else
            high = middle;
    }
    return low;
}

// The maximal number of points that the left part keeps on the jump stack at one moment.
static size_t left_part_max_depth(const T_OPERATOR_BUFFER *buf)
{
    size_t depth = 0;
    size_t max_depth = 0;
    for (size_t index = 0; index < buf->statement_count; index++)
    {
        const uint8_t operator = statement_code(buf, index);
        if (is_open_statement(operator))
        {
            depth++;
            if (depth > max_depth)
                max_depth = depth;
        }
        else if (operator == n_eoe || operator == n_eoei)
        {
            const size_t removed = removed_points(buf, index);
            depth = (removed >= depth) ? 0 : depth - removed;
        }
    }
    return max_depth;
}

// The index of the statement after the group that begins with the "open" statement `first`: the group
// ends where n_eoe/n_eoei removes the last of the points put by the group. Zero is returned when the group
// is not closed before the end of the left part, removes more points than it has put, or contains
// a statement that is not expected in a search.
static size_t group_end(const T_OPERATOR_BUFFER *buf, size_t first)
{
    size_t depth = 0;
    for (size_t index = first; index + 1 < buf->statement_count; index++) // the last statement is n_eor
    {
        const uint8_t operator = statement_code(buf, index);
        if (is_open_statement(operator))
            depth++;
        else if (operator == n_eoe || operator == n_eoei)
        {
            const size_t removed = removed_points(buf, index);
            if (removed > depth)
                return 0;
            depth -= removed;
            if (depth == 0)
                return index + 1;
        }
        else if (!is_continue_statement(operator) && !is_simple_statement(operator))
            return 0;
    }
    return 0;
}

// Appends the term made of the statements [first, end) to the terms of a sentence.
static void add_term(const T_OPERATOR_BUFFER *buf, T_SENTENCE_TERMS *info, size_t first, size_t end)
{
    T_TERM *term = &info->items[info->item_count++];
    term->offset = buf->statements[first];
    term->end = (end < buf->statement_count) ? buf->statements[end] : buf->length;
    term->label_first = first_label_at(buf, term->offset);
    term->label_end = first_label_at(buf, term->end);
    return;
}

// Cuts the left part of a sentence into terms; the terms are taken from the beginning
// until the first statement that cannot be merged.
static void split_into_terms(const T_STORED_SENTENCE *sentence, T_SENTENCE_TERMS *info)
{
    const T_OPERATOR_BUFFER *buf = &sentence->left_part;
    const size_t count = buf->statement_count;
    info->items = NULL;
    info->item_count = 0;
    info->tail_offset = 0;
    info->tail_label_first = 0;
    info->max_depth = left_part_max_depth(buf);
    // Without n_eor at the end the left part is not what the compiler generates: nothing is merged.
    if (count == 0 || statement_code(buf, count - 1) != n_eor)
        return;
    info->items = (T_TERM *)calloc(count, sizeof(T_TERM));
    if (info->items == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(split_into_terms): items=%p count=%zu\n", (void *)info->items, count);
#endif
    size_t index = 0;
    while (index + 1 < count)
    {
        const uint8_t operator = statement_code(buf, index);
        if (is_simple_statement(operator))
        {
            add_term(buf, info, index, index + 1);
            index++;
        }
        else if (is_open_statement(operator))
        {
            const size_t end = group_end(buf, index);
            if (end == 0)
                break;
            add_term(buf, info, index, end);
            index = end;
        }
        else
            break;
    }
    info->tail_offset = buf->statements[index];
    info->tail_label_first = first_label_at(buf, info->tail_offset);
    return;
}

// Two terms are equal when they consist of the same bytes and refer to the same labels
// (an alias of a label is the same as the label itself).
static bool terms_equal(const T_STORED_SENTENCE *a, const T_TERM *term_a, const T_STORED_SENTENCE *b, const T_TERM *term_b)
{
    const size_t length = term_a->end - term_a->offset;
    if (length != term_b->end - term_b->offset)
        return false;
    if (memcmp(a->left_part.bytes + term_a->offset, b->left_part.bytes + term_b->offset, length) != 0)
        return false;
    const size_t label_count = term_a->label_end - term_a->label_first;
    if (label_count != term_b->label_end - term_b->label_first)
        return false;
    for (size_t i = 0; i < label_count; i++)
    {
        const T_LABEL_REF *ref_a = &a->left_part.labels[term_a->label_first + i];
        const T_LABEL_REF *ref_b = &b->left_part.labels[term_b->label_first + i];
        if (ref_a->offset - term_a->offset != ref_b->offset - term_b->offset)
            return false;
        if (resolve_label_alias(ref_a->label) != resolve_label_alias(ref_b->label))
            return false;
    }
    return true;
}

#if defined mdebug
// Prints the first statement of a term and, for a group, the statement that closes it (with its operand).
static void debug_print_term(const T_MERGE_CONTEXT *ctx, size_t sentence_index, size_t p)
{
    const T_OPERATOR_BUFFER *buf = &ctx->sentences[sentence_index]->left_part;
    const T_TERM *term = &ctx->terms[sentence_index].items[p];
    fprintf(stderr, " %s", operator_name(buf->bytes[term->offset]));
    if (is_open_statement(buf->bytes[term->offset]))
    {
        size_t last = 0;
        for (size_t index = 0; index < buf->statement_count && buf->statements[index] < term->end; index++)
            last = index;
        fprintf(stderr, "..%s", operator_name(statement_code(buf, last)));
        if (statement_code(buf, last) == n_eoe)
            fprintf(stderr, "(%zu)", removed_points(buf, last));
    }
    fprintf(stderr, "\n");
    return;
}
#endif

// Writes the term number p of the sentence.
static void write_term(const T_MERGE_CONTEXT *ctx, size_t sentence_index, size_t p)
{
    const T_OPERATOR_BUFFER *buf = &ctx->sentences[sentence_index]->left_part;
    const T_TERM *term = &ctx->terms[sentence_index].items[p];
    write_range_to_macrocode(buf, term->offset, term->end, term->label_first);
    return;
}

// Writes the sentence from the term number p: the rest of its left part and the whole right part.
static void write_sentence_from(const T_MERGE_CONTEXT *ctx, size_t sentence_index, size_t p)
{
    const T_STORED_SENTENCE *sentence = ctx->sentences[sentence_index];
    const T_SENTENCE_TERMS *info = &ctx->terms[sentence_index];
    size_t offset = info->tail_offset;
    size_t label_first = info->tail_label_first;
    if (p < info->item_count)
    {
        offset = info->items[p].offset;
        label_first = info->items[p].label_first;
    }
    write_range_to_macrocode(&sentence->left_part, offset, sentence->left_part.length, label_first);
    write_buffer_to_macrocode(&sentence->right_part);
    return;
}

// The end of the run of the sentences [first, limit) that have the same term number p
// as the sentence `first`. A sentence without such a term makes a run of its own.
static size_t run_end(const T_MERGE_CONTEXT *ctx, size_t first, size_t limit, size_t p)
{
    size_t end = first + 1;
    if (p >= ctx->terms[first].item_count)
        return end;
    while (end < limit && p < ctx->terms[end].item_count &&
           terms_equal(ctx->sentences[first], &ctx->terms[first].items[p], ctx->sentences[end], &ctx->terms[end].items[p]))
        end++;
    return end;
}

// Writes the sentences [low, high), which are known to have the same first p terms (already written).
// level is the number of the enclosing nodes with several children, each of them may keep one n_sjump
// point on the jump stack.
static void write_node(const T_MERGE_CONTEXT *ctx, size_t low, size_t high, size_t p, size_t level)
{
    if (high - low == 1)
    {
        write_sentence_from(ctx, low, p);
        return;
    }
    // All the sentences have the same term: it is written once, there is nothing to choose between.
    if (run_end(ctx, low, high, p) == high)
    {
#if defined mdebug
        fprintf(stderr, "write_node: common term %zu of %zu sentences (level %zu)", p, high - low, level);
        debug_print_term(ctx, low, p);
#endif
        write_term(ctx, low, p);
        write_node(ctx, low, high, p + 1, level);
        return;
    }
    // The children of the node are the runs of the sentences with the same term and the sentences
    // that have nothing to merge. The first child follows the statements of the node directly,
    // for each next child the n_sjump statement of the previous one is a return point.
    size_t first = low;
    while (first < high)
    {
        size_t end = run_end(ctx, first, high, p);
        // A run inside of a node may add one more point to the jump stack, which must be limited.
        if (end - first >= 2 && level + 1 >= ctx->max_levels)
            end = first + 1;
        if (first > low)
            macrocode_label(ctx->sentences[first - 1]->sentence_label);
        if (end < high)
            generate_operator_l(n_sjump, ctx->sentences[end - 1]->sentence_label);
        if (end - first >= 2)
        {
#if defined mdebug
            fprintf(stderr, "write_node: merged %zu sentences by the term %zu (level %zu)", end - first, p, level);
            debug_print_term(ctx, first, p);
#endif
            write_term(ctx, first, p);
            write_node(ctx, first, end, p + 1, level + 1);
        }
        else
            write_sentence_from(ctx, first, p);
        first = end;
    }
    return;
}

// Writes the sentences of a function: the left parts are merged where they begin in the same way.
static void write_function_body(const T_STORED_FUNCTION *func)
{
    if (func->sentences == NULL)
        return;
    const size_t count = func->sentence_count;
    T_MERGE_CONTEXT ctx;
    ctx.sentences = (T_STORED_SENTENCE **)calloc(count, sizeof(T_STORED_SENTENCE *));
    ctx.terms = (T_SENTENCE_TERMS *)calloc(count, sizeof(T_SENTENCE_TERMS));
    if (ctx.sentences == NULL || ctx.terms == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(write_function_body): sentences=%p terms=%p count=%zu\n", (void *)ctx.sentences, (void *)ctx.terms, count);
#endif
    size_t max_depth = 0;
    size_t index = 0;
    for (T_STORED_SENTENCE *sentence = func->sentences; sentence != NULL; sentence = sentence->next)
    {
        ctx.sentences[index] = sentence;
        split_into_terms(sentence, &ctx.terms[index]);
        if (ctx.terms[index].max_depth > max_depth)
            max_depth = ctx.terms[index].max_depth;
        index++;
    }
    // One point of the jump stack is always spent by the node with several children; the rest is left
    // for the searches of the left parts and for the nested nodes.
    ctx.max_levels = (max_depth + 1 >= JUMP_STACK_SIZE) ? 1 : JUMP_STACK_SIZE - 1 - max_depth;
    if (ctx.max_levels > MAX_MERGE_LEVELS)
        ctx.max_levels = MAX_MERGE_LEVELS;
#if defined mdebug
    fprintf(stderr, "write_function_body: sentences=%zu max_depth=%zu max_levels=%zu\n", count, max_depth, ctx.max_levels);
#endif
    write_node(&ctx, 0, index, 0, 0);
    for (size_t i = 0; i < index; i++)
    {
        if (ctx.terms[i].items != NULL)
        {
#if defined mdebug
            fprintf(stderr, "free(write_function_body): items=%p\n", (void *)ctx.terms[i].items);
#endif
            free(ctx.terms[i].items);
        }
    }
#if defined mdebug
    fprintf(stderr, "free(write_function_body): sentences=%p terms=%p\n", (void *)ctx.sentences, (void *)ctx.terms);
#endif
    free(ctx.terms);
    free(ctx.sentences);
    return;
}

// Writes every function to the macrocode: the name, the function label and the sentences.
// The sentences are written by write_function_body: the left parts that begin in the same way
// are merged, the n_sjump statements before the sentences (or the groups of sentences) are
// the points to return to after a failure. The last sentence of a function ends with the failure
// of the whole function, which needs no statement: it is the failure with the empty jump stack.
void function_pool_finalize(void)
{
#if defined mdebug
    fprintf(stderr, "function_pool_finalize: begin, head=%p tail=%p\n", (void *)function_pool_head, (void *)function_pool_tail);
#endif
    compile_output_set_mode(OUTPUT_MACROCODE, NULL);
    T_STORED_FUNCTION *func = function_pool_head;
#if defined mdebug
    int func_count = 0;
#endif
    while (func != NULL)
    {
#if defined mdebug
        func_count++;
        fprintf(stderr, "  function %d: func=%p label=%p sentences=%zu\n", func_count, (void *)func, (void *)func->func_label, func->sentence_count);
#endif
        write_function_name(func);
        macrocode_label(func->func_label);
        write_function_body(func);
        func = func->next;
    }
#if defined mdebug
    fprintf(stderr, "function_pool_finalize: end, %d functions processed\n", func_count);
#endif
    return;
}

//----------  end of file function_pool.c  ----------
