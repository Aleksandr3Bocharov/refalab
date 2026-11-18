// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file macrocode.c  ----------
//             generator of macrocode text
//----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include "refalab.h"
#include "macrocode.h"
#include "print_errors.h"
#include "compiler.h"
#include "avl_identifiers.h"

// Error: an external name is already used by an entry or an extrn directive.
#define PRINT_ERROR_604 \
    print_error_three_strings(604, "External label", identifier_extern, identifier_extern_length, " is already defined")

// Exported (entry) label: the label with its external name; a singly linked list.
typedef struct entry
{ // entry table element
    struct entry *next;
    T_LABEL *label;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_ENTRY;

// Imported (extrn) label: the label with its external name; a singly linked list.
// The label's info.infon holds the ordinal number of the extern in this list.
typedef struct extrn
{ // external pointer table element
    struct extrn *next;
    T_LABEL *label;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_EXTRN;

// Description of a piece of the module image: `delta` plain bytes followed by
// the address of `label` (a NULL label marks the end of the image).
typedef struct relay
{
    T_LABEL *label;
    size_t delta;
} T_RELAY;

// A growing memory buffer used as a sequential stream (length - size of the buffer,
// current - read/write position).
typedef struct stream_bytes_nodes
{
    size_t length;
    size_t current;
    uint8_t *buffer;
} T_STREAM_BYTES_NODES;

// The image is kept in two streams: stream_bytes - the plain bytes,
// stream_nodes - the T_RELAY records telling where the addresses go between the bytes.
static T_STREAM_BYTES_NODES stream_bytes = {0, 0, NULL};
static T_STREAM_BYTES_NODES stream_nodes = {0, 0, NULL};

static T_ENTRY *entry, *entry2;
static T_EXTRN *extrn, *extrn2;

static T_ENTRY *first_entry;
static T_ENTRY *last_entry;
static T_EXTRN *first_extrn;
static T_EXTRN *last_extrn;
static size_t extrn_count;
// Offset of the next byte of the image from its start.
static size_t current_address; // module generation files
static size_t module_length;
// The record being written or read, and the number of plain bytes since the last address.
static T_RELAY relay;
static size_t delta;

// Releases the buffer of a stream.
static inline void stream_bytes_nodes_clear(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
#if defined mdebug
    fprintf(stderr, "free(stream_bytes_nodes_clear): stream_bytes_nodes->buffer=%p\n", (void *)stream_bytes_nodes->buffer);
#endif
    free(stream_bytes_nodes->buffer);
    stream_bytes_nodes->buffer = NULL;
    stream_bytes_nodes->length = 0;
    stream_bytes_nodes->current = 0;
    return;
}

// Allocates the initial buffer: tries 8192 records and halves the size until
// the allocation succeeds (but not below 2 records).
static void stream_bytes_nodes_new(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    stream_bytes_nodes_clear(stream_bytes_nodes);
    const size_t max_stream_bytes_nodes_length = 8192 * sizeof(T_RELAY);
    const size_t min_stream_bytes_nodes_length = 2 * sizeof(T_RELAY);
    size_t stream_bytes_nodes_length = max_stream_bytes_nodes_length;
    while (true)
    {
        stream_bytes_nodes->buffer = (uint8_t *)malloc(stream_bytes_nodes_length);
        if (stream_bytes_nodes->buffer != NULL)
        {
            stream_bytes_nodes->length = stream_bytes_nodes_length;
#if defined mdebug
            fprintf(stderr, "malloc(stream_bytes_nodes_new): stream_bytes_nodes->buffer=%p stream_bytes_nodes->length=%zu\n", (void *)stream_bytes_nodes->buffer, stream_bytes_nodes->length);
#endif
            break;
        }
        else
        {
            stream_bytes_nodes_length /= 2;
            if (stream_bytes_nodes_length < min_stream_bytes_nodes_length)
                error_no_memory();
        }
    } // while
    return;
}

// Enlarges the buffer by up to 8192 records, halving the step on a lack of memory.
static void stream_bytes_nodes_append(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    const size_t max_stream_bytes_nodes_append_length = 8192 * sizeof(T_RELAY);
    const size_t min_stream_bytes_nodes_append_length = 2 * sizeof(T_RELAY);
    size_t stream_bytes_nodes_append_length = max_stream_bytes_nodes_append_length;
    while (true)
    {
        const size_t new_length = stream_bytes_nodes->length + stream_bytes_nodes_append_length;
        uint8_t *new_buffer = (uint8_t *)realloc(stream_bytes_nodes->buffer, new_length);
        if (new_buffer != NULL)
        {
            stream_bytes_nodes->buffer = new_buffer;
            stream_bytes_nodes->length = new_length;
#if defined mdebug
            fprintf(stderr, "realloc(stream_bytes_nodes_append): stream_bytes_nodes->buffer=%p stream_bytes_nodes->length=%zu\n", (void *)stream_bytes_nodes->buffer, stream_bytes_nodes->length);
#endif
            break;
        }
        else
        {
            stream_bytes_nodes_append_length /= 2;
            if (stream_bytes_nodes_append_length < min_stream_bytes_nodes_append_length)
                error_no_memory();
        }
    } // while
    return;
}

// Rewinds a stream to read it from the beginning.
static inline void stream_bytes_nodes_begin(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    stream_bytes_nodes->current = 0;
    return;
}

// Appends the current relay record to stream_nodes.
static void stream_nodes_write(void)
{
    while (true)
    {
        const size_t residual = stream_nodes.length - stream_nodes.current;
        if (residual >= sizeof(T_RELAY))
        {
            memcpy(stream_nodes.buffer + stream_nodes.current, &relay, sizeof(T_RELAY));
            stream_nodes.current += sizeof(T_RELAY);
            return;
        }
        stream_bytes_nodes_append(&stream_nodes);
    } // while
} // stream_nodes_write

// Reads the next byte of stream_bytes.
static inline void stream_bytes_read(uint8_t *buffer)
{
    if (stream_bytes.current != stream_bytes.length)
    {
        memcpy(buffer, stream_bytes.buffer + stream_bytes.current, 1);
        stream_bytes.current++;
    }
    return;
} // stream_bytes_read

// Reads the next relay record from stream_nodes.
static inline void stream_nodes_read(void)
{
    const size_t residual = stream_nodes.length - stream_nodes.current;
    if (residual >= sizeof(T_RELAY))
    {
        memcpy(&relay, stream_nodes.buffer + stream_nodes.current, sizeof(T_RELAY));
        stream_nodes.current += sizeof(T_RELAY);
    }
    return;
} // stream_nodes_read

// Begins a new module image: empty streams and empty entry/extrn lists
// (each list starts with a dummy head node).
void macrocode_start(void)
{
    delta = 0;
    stream_bytes_nodes_new(&stream_bytes);
    stream_bytes_nodes_new(&stream_nodes);
    first_entry = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (first_entry == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_start): first_entry=%p\n", (void *)first_entry);
#endif
    last_entry = first_entry;
    first_entry->next = NULL;
    first_extrn = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (first_extrn == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_start): first_extrn=%p\n", (void *)first_extrn);
#endif
    last_extrn = first_extrn;
    first_extrn->next = NULL;
    current_address = 0;
    extrn_count = 1;
    return;
} // macrocode_start

// The current address (offset) in the image.
size_t macrocode_where(void)
{
    return current_address;
}

// Appends a byte to the image.
void macrocode_byte(uint8_t byte)
{
#if defined mdebug
    fprintf(stderr, "macrocode_byte: byte=0x%02X\n", byte);
#endif
    if (stream_bytes.current != stream_bytes.length)
    {
        *(stream_bytes.buffer + stream_bytes.current) = byte;
        stream_bytes.current++;
    }
    else
    {
        stream_bytes_nodes_append(&stream_bytes);
        *(stream_bytes.buffer + stream_bytes.current) = byte;
        stream_bytes.current++;
    }
    delta++;
    current_address++;
    return;
} // macrocode_byte

// Appends an address slot of a label: the label and the number of plain bytes before it
// are recorded, the slot itself is produced when the image is written.
void macrocode_address(T_LABEL *label)
{
    relay.label = label;
    relay.delta = delta;
    delta = 0;
    stream_nodes_write();
    current_address += LBLL;
    return;
}

// Registers an exported label. An external name that was already used
// is error 604.
void macrocode_entry(T_LABEL *label, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
// identifier_extern label
{
    // identifier_extern_length label length
    extrn2 = first_extrn;
    while (extrn2 != last_extrn)
    {
        extrn2 = extrn2->next;
        if (extrn2->identifier_extern_length == identifier_extern_length && strncmp(extrn2->identifier_extern, identifier_extern, identifier_extern_length) == 0)
        {
            scanner.last_error_cursor = identifier_extern_cursor_number;
            PRINT_ERROR_604;
            return;
        }
    }
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == identifier_extern_length && strncmp(entry2->identifier_extern, identifier_extern, identifier_extern_length) == 0)
        {
            scanner.last_error_cursor = identifier_extern_cursor_number;
            PRINT_ERROR_604;
            return;
        }
    }
    entry2 = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (entry2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_entry): entry2=%p\n", (void *)entry2);
#endif
    last_entry->next = entry2;
    last_entry = entry2;
    entry2->label = label;
    entry2->next = NULL;
    entry2->identifier_extern_length = identifier_extern_length;
    strncpy(entry2->identifier_extern, identifier_extern, entry2->identifier_extern_length);
    label->mode |= 0040;
    return;
} // macrocode_entry

// Registers an imported label. The same external name imported twice shares one number;
// an external name that is exported by this module is error 604.
void macrocode_extrn(T_LABEL *label, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
// identifier_extern label
{
    // identifier_extern_length label length
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == identifier_extern_length && strncmp(entry2->identifier_extern, identifier_extern, identifier_extern_length) == 0)
        {
            scanner.last_error_cursor = identifier_extern_cursor_number;
            PRINT_ERROR_604;
            return;
        }
    }
    extrn2 = first_extrn;
    while (extrn2 != last_extrn)
    {
        extrn2 = extrn2->next;
        if (extrn2->identifier_extern_length == identifier_extern_length && strncmp(extrn2->identifier_extern, identifier_extern, identifier_extern_length) == 0)
        {
            label->info.infon = extrn2->label->info.infon;
            label->mode |= 0220;
            return;
        }
    }
    extrn2 = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (extrn2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_extrn): extrn2=%p\n", (void *)extrn2);
#endif
    last_extrn->next = extrn2;
    last_extrn = extrn2;
    extrn2->label = label;
    extrn2->next = NULL;
    extrn2->identifier_extern_length = identifier_extern_length;
    strncpy(extrn2->identifier_extern, identifier_extern, extrn2->identifier_extern_length);
    label->mode |= 0220;
    extrn_count++;
    label->info.infon = extrn_count;
    return;
} // macrocode_extrn

// Defines the label (internal, defined) at the current address.
void macrocode_label(T_LABEL *label)
{
#if defined mdebug
    fprintf(stderr, "macrocode_label: label=%p type=%d mode=%d current_address=%zu\n", (void *)label, label->type, label->mode, current_address);
#endif
    label->mode |= 0120;
    label->info.infon = current_address;
    return;
}

// Makes equ_label an alias of label.
void macrocode_equ(T_LABEL *equ_label, T_LABEL *label)
{
    equ_label->info.infop = label;
    equ_label->mode |= 0320;
    return;
}

// Writes the final relay record (without a label) and fixes the module length.
static inline void ending(void)
{
    relay.delta = delta;
    relay.label = NULL;
    stream_nodes_write();
    module_length = current_address;
    return;
} // ending

// Checks the result of an output call; a write error terminates the compiler (code 8).
static inline void write_llvm_source(int put_result)
{
    if (put_result == EOF)
        if (feof(llvm_source) != 0 || ferror(llvm_source) != 0)
        {
            printf("Write i/o error in llvm source file\n");
            exit(8);
        }
}

// Writes the module image as LLVM IR: one packed global structure @_d<N>$ that
// mixes byte arrays and pointers, then the declarations of the imported labels
// and the aliases of the exported ones. Names are made lower case with the
// "refalab_" prefix. Pass 1 of the structure writes its type, pass 2 its values:
// byte arrays as constants, addresses of internal labels as offsets in the structure,
// addresses of imported labels as references to the imported globals.
// Afterwards all streams and lists are released.
void macrocode_end(void)
{
    ending();
    uint8_t byte = 0;
    char buffer_string[1024]; // Increased buffer size for safe formatting of long identifiers
    // Module header. Using packed structure syntax <{ ... }>
    // to prevent LLVM from inserting unexpected padding between bytes and pointers.
    sprintf(buffer_string, "@_d%" PRIu32 "$ = private global <{\n", scanner.module_number);
    write_llvm_source(fputs(buffer_string, llvm_source));
    if (module_length != 0)
    {
        // ====================================================================
        // PASS 1: Generate the exact type map of the packed structure
        // ====================================================================
        stream_bytes_nodes_begin(&stream_bytes);
        stream_bytes_nodes_begin(&stream_nodes);
        bool first_element = true;
        while (true)
        {
            stream_nodes_read();
            delta = relay.delta;
            // If there are raw bytes preceding a label/relay
            if (delta > 0)
            {
                if (!first_element)
                    write_llvm_source(fputs(",\n", llvm_source));
                sprintf(buffer_string, "\t[%zu x i8]", delta);
                write_llvm_source(fputs(buffer_string, llvm_source));
                first_element = false;
                // Advance the byte stream pointer forward
                for (size_t k = 0; k < delta; k++)
                    stream_bytes_read(&byte);
            }
            const T_LABEL *label = relay.label;
            if (label != NULL)
            {
                if (!first_element)
                    write_llvm_source(fputs(",\n", llvm_source));
                write_llvm_source(fputs("\tptr", llvm_source));
                first_element = false;
                continue;
            }
            break;
        }
        // Close type block <{ }>, open values block <{ }>
        write_llvm_source(fputs("\n}> <{\n", llvm_source));
        // ====================================================================
        // PASS 2: Populate the structure with constant values
        // ====================================================================
        stream_bytes_nodes_begin(&stream_bytes);
        stream_bytes_nodes_begin(&stream_nodes);
        first_element = true;
        while (true)
        {
            stream_nodes_read();
            delta = relay.delta;
            if (delta > 0)
            {
                if (!first_element)
                    write_llvm_source(fputs(",\n", llvm_source));
                sprintf(buffer_string, "\t[%zu x i8] c\"", delta);
                write_llvm_source(fputs(buffer_string, llvm_source));
                for (size_t k = 0; k < delta; k++)
                {
                    stream_bytes_read(&byte);
                    // Hexadecimal byte escaping according to the LLVM IR standard (\XX)
                    sprintf(buffer_string, "\\%02X", byte);
                    write_llvm_source(fputs(buffer_string, llvm_source));
                }
                write_llvm_source(fputs("\"", llvm_source));
                first_element = false;
            }
            T_LABEL *label = relay.label;
            if (label != NULL)
            {
                if (!first_element)
                    write_llvm_source(fputs(",\n", llvm_source));
                label = resolve_label_alias(label);
                if ((label->mode & 0300) != 0200)
                {
                    sprintf(buffer_string, "\tptr getelementptr (i8, ptr @_d%" PRIu32 "$, i%zu %zu)", scanner.module_number, LBLL * 8, label->info.infon);
                    write_llvm_source(fputs(buffer_string, llvm_source));
                }
                else
                {
                    write_llvm_source(fputs("\tptr @refalab_", llvm_source));
                    extrn = first_extrn;
                    for (size_t i = 1; i < label->info.infon; i++)
                        extrn = extrn->next;
                    for (uint8_t i = 0; i < extrn->identifier_extern_length; i++)
                        write_llvm_source(fputc(tolower((unsigned char)*(extrn->identifier_extern + i)), llvm_source));
                }
                first_element = false;
                continue;
            }
            break;
        }
        write_llvm_source(fputs("\n}>,\n", llvm_source));
        // Define global variable alignment based on pointer size (LBLL)
        sprintf(buffer_string, "align %zu\n\n", LBLL);
        write_llvm_source(fputs(buffer_string, llvm_source));
        // ====================================================================
        // IMPORTS: Global declarations for external labels
        // ====================================================================
        extrn = first_extrn->next;
        while (extrn != NULL)
        {
            write_llvm_source(fputs("@refalab_", llvm_source));
            for (uint8_t i = 0; i < extrn->identifier_extern_length; i++)
                write_llvm_source(fputc(tolower((unsigned char)*(extrn->identifier_extern + i)), llvm_source));
            write_llvm_source(fputs(" = external global i8\n", llvm_source));
            extrn = extrn->next;
        }
        write_llvm_source(fputs("\n", llvm_source));
        // ====================================================================
        // EXPORTS: Global entry points exposed via LLVM aliases
        // ====================================================================
        entry = first_entry->next;
        while (entry != NULL)
        {
            const T_LABEL *label = resolve_label_alias(entry->label);
            write_llvm_source(fputs("@refalab_", llvm_source));
            for (uint8_t i = 0; i < entry->identifier_extern_length; i++)
                write_llvm_source(fputc(tolower((unsigned char)*(entry->identifier_extern + i)), llvm_source));
            // dso_local alias binds the public symbol to a specific byte offset inside the module data
            sprintf(buffer_string, " = dso_local alias i8, getelementptr (i8, ptr @_d%" PRIu32 "$, i%zu %zu)\n", scanner.module_number, LBLL * 8, label->info.infon);
            write_llvm_source(fputs(buffer_string, llvm_source));
            entry = entry->next;
        }
    }
    // ====================================================================
    // TERMINATION: Stream cleanup and dynamic memory deallocation
    // ====================================================================
    stream_bytes_nodes_clear(&stream_bytes);
    stream_bytes_nodes_clear(&stream_nodes);
    entry = first_entry;
    while (entry != NULL)
    {
        entry2 = entry->next;
#if defined mdebug
        fprintf(stderr, "free(macrocode_end) entry=%p\n", (void *)entry);
#endif
        free(entry);
        entry = entry2;
    }
    extrn = first_extrn;
    while (extrn != NULL)
    {
        extrn2 = extrn->next;
#if defined mdebug
        fprintf(stderr, "free(macrocode_end) extrn=%p\n", (void *)extrn);
#endif
        free(extrn);
        extrn = extrn2;
    }
    return;
}

//----------  end of file macrocode.c  ----------
