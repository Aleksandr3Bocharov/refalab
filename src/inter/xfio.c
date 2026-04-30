// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-30
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  XFIO.C ---------------
//           MO: file input/output
//           MO: file/dir remove/rename/exist
//---------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "refalab.h"
#include "rfintf.h"

#define FILES_MAX 10

extern uint8_t refalab_true, refalab_false;
extern uint8_t refalab_null;
extern uint8_t refalab_begin, refalab_end, refalab_cur;

static FILE *file;
static FILE *files[FILES_MAX] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

char feof_0[] = {Z4 'F', 'E', 'O', 'F', (char)4};
G_L_B uint8_t refalab_feof = '\002';

char ferror_0[] = {Z6 'F', 'E', 'R', 'R', 'O', 'R', (char)6};
G_L_B uint8_t refalab_ferror = '\002';

char stdin_0[] = {Z5 'S', 'T', 'D', 'I', 'N', (char)5};
G_L_B uint8_t refalab_stdin = '\002';

char stdout_0[] = {Z6 'S', 'T', 'D', 'O', 'U', 'T', (char)6};
G_L_B uint8_t refalab_stdout = '\002';

char stderr_0[] = {Z6 'S', 'T', 'D', 'E', 'R', 'R', (char)6};
G_L_B uint8_t refalab_stderr = '\002';

static void fopen_(void)
{
    do
    {
        T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGO)
            break;
        char symbol = current_argument->info.infoc;
        char file_mode[3];
        file_mode[1] = '\0';
        file_mode[2] = '\0';
        if (symbol == 'R' || symbol == 'r')
            file_mode[0] = 'r';
        else if (symbol == 'W' || symbol == 'w')
            file_mode[0] = 'w';
        else if (symbol == 'A' || symbol == 'a')
            file_mode[0] = 'a';
        else
            break;
        current_argument = current_argument->next;
        if (current_argument->tag == TAGO)
        {
            symbol = current_argument->info.infoc;
            if (symbol == 'B' || symbol == 'b')
                file_mode[1] = 'b';
            else
                break;
            current_argument = current_argument->next;
        }
        if (current_argument->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(current_argument);
        if (file_number >= FILES_MAX)
            break;
        current_argument = current_argument->next;
        char file_name[MAX_PATHFILENAME + 1];
        current_argument = get_string_expression(file_name, MAX_PATHFILENAME, current_argument, refal.next_argument);
        if (current_argument != refal.next_argument)
            break;
        file = fopen(file_name, file_mode);
        files[file_number] = file;
        if (file == NULL)
        {
            const int error_number = errno;
            const char *string_error = strerror(error_number);
            const int32_t result_yet_need = (int32_t)strlen(string_error) - ((int32_t)strlen(file_mode) + (int32_t)strlen(file_name) + 2);
            if (result_yet_need > 0)
                if (!extended_insert_from_free_memory_list(refal.next_result, (size_t)result_yet_need))
                    return;
            current_argument = set_string_expression(string_error, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, current_argument->next);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fopen_0[] = {Z5 'F', 'O', 'P', 'E', 'N', (char)5};
G_L_B uint8_t refalab_fopen = '\122';
void (*fopen_1)(void) = fopen_;

static void fclose_(void)
{
    do
    {
        T_LINKCB *symbol_number = refal.previous_argument->next;
        if (symbol_number->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(symbol_number);
        if (file_number >= FILES_MAX)
            break;
        if (symbol_number->next != refal.next_argument)
            break;
        file = files[file_number];
        files[file_number] = NULL;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        const int close_result = fclose(file);
        if (close_result == EOF)
        {
            const int error_number = errno;
            const char *string_error = strerror(error_number);
            if (!extended_insert_from_free_memory_list(refal.next_result, strlen(string_error) - 2))
                return;
            set_string_expression(string_error, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, refal.next_argument);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fclose_0[] = {Z6 'F', 'C', 'L', 'O', 'S', 'E', (char)6};
G_L_B uint8_t refalab_fclose = '\122';
void (*fclose_1)(void) = fclose_;

static void fgets_(void)
{
    do
    {
        const T_LINKCB *argument = refal.previous_argument->next;
        if (argument->tag == TAGN)
        {
            const uint32_t file_number = gcoden(argument);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (argument->tag == TAGF)
        {
            if (argument->info.codef != &refalab_stdin)
                break;
            file = stdin;
        }
        else
            break;
        if (argument->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        T_LINKCB *current_result = refal.previous_result;
        int get_result = getc(file);
        while (get_result != '\n')
        {
            if (!extended_insert_from_free_memory_list(current_result, 1))
                return;
            current_result = current_result->next;
            current_result->info.code = NULL;
            if (set_eof_linkcb(get_result, file, current_result))
                return;
            current_result->tag = TAGO;
            current_result->info.infoc = (char)get_result;
            get_result = getc(file);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fgets_0[] = {Z5 'F', 'G', 'E', 'T', 'S', (char)5};
G_L_B uint8_t refalab_fgets = '\122';
void (*fgets_1)(void) = fgets_;

static void fputs_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag == TAGN)
        {
            const uint32_t file_number = gcoden(current_argument);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (current_argument->tag == TAGF)
        {
            if (current_argument->info.codef == &refalab_stdout)
                file = stdout;
            else if (current_argument->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        const T_LINKCB *temp_argument = current_argument->next;
        bool impossible = false;
        while (temp_argument != refal.next_argument)
        {
            if (temp_argument->tag != TAGO)
            {
                impossible = true;
                break;
            }
            temp_argument = temp_argument->next;
        }
        if (impossible)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        current_argument = current_argument->next;
        while (current_argument != refal.next_argument)
        {
            const int symbol = current_argument->info.infoc;
            const int put_result = putc(symbol, file);
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
            {
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
                return;
            }
            current_argument = current_argument->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fputs_0[] = {Z5 'F', 'P', 'U', 'T', 'S', (char)5};
G_L_B uint8_t refalab_fputs = '\122';
void (*fputs_1)(void) = fputs_;

static void fprint_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag == TAGN)
        {
            const uint32_t file_number = gcoden(current_argument);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (current_argument->tag == TAGF)
        {
            if (current_argument->info.codef == &refalab_stdout)
                file = stdout;
            else if (current_argument->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        current_argument = current_argument->next;
        while (current_argument != refal.next_argument)
        {
            int put_result = 0;
            char string[512];
            if (current_argument->tag == TAGO)
                put_result = putc(current_argument->info.infoc, file);
            else if (current_argument->tag == TAGLB)
                put_result = putc('(', file);
            else if (current_argument->tag == TAGRB)
                put_result = putc(')', file);
            else if (current_argument->tag == TAGN)
            {
                sprintf(string, "'%u'", gcoden(current_argument));
                put_result = fputs(string, file);
            }
            else if (current_argument->tag == TAGF)
            {
                sprintf(string, "'");
                const uint8_t *label_length = current_argument->info.codef - 1;
                const char *label = (char *)label_length - *label_length;
                char symbol[2];
                symbol[1] = '\0';
                for (uint8_t i = 1; i <= *label_length; i++, label++)
                {
                    symbol[0] = (char)toupper(*label);
                    strcat(string, symbol);
                }
                strcat(string, "'");
                put_result = fputs(string, file);
            }
            else if (current_argument->tag == TAGR)
            {
                sprintf(string, "'%%%p'", (void *)current_argument->info.codep);
                put_result = fputs(string, file);
            }
            else if (BRACKET(current_argument))
                refal_abort_end("fprint: unknown bracket type");
            else
            {
                sprintf(string, "'%x,%p'", current_argument->tag, current_argument->info.code);
                put_result = fputs(string, file);
            }
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
            {
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
                return;
            }
            current_argument = current_argument->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprint_0[] = {Z6 'F', 'P', 'R', 'I', 'N', 'T', (char)6};
G_L_B uint8_t refalab_fprint = '\122';
void (*fprint_1)(void) = fprint_;

static void fprints_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag == TAGN)
        {
            const uint32_t file_number = gcoden(current_argument);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (current_argument->tag == TAGF)
        {
            if (current_argument->info.codef == &refalab_stdout)
                file = stdout;
            else if (current_argument->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        current_argument = current_argument->next;
        while (current_argument != refal.next_argument)
        {
            int put_result = 0;
            char string[512];
            if (current_argument->tag == TAGO)
                put_result = putc(current_argument->info.infoc, file);
            else if (current_argument->tag == TAGLB)
                put_result = putc('(', file);
            else if (current_argument->tag == TAGRB)
                put_result = putc(')', file);
            else if (current_argument->tag == TAGN)
            {
                sprintf(string, "%u", gcoden(current_argument));
                put_result = fputs(string, file);
            }
            else if (current_argument->tag == TAGF)
            {
                string[0] = '\0';
                const uint8_t *label_length = current_argument->info.codef - 1;
                const char *label = (char *)label_length - *label_length;
                char symbol[2];
                symbol[1] = '\0';
                for (uint8_t i = 1; i <= *label_length; i++, label++)
                {
                    symbol[0] = (char)toupper(*label);
                    strcat(string, symbol);
                }
                put_result = fputs(string, file);
            }
            else if (current_argument->tag == TAGR)
            {
                sprintf(string, "%%%p", (void *)current_argument->info.codep);
                put_result = fputs(string, file);
            }
            else if (BRACKET(current_argument))
                refal_abort_end("fprints: unknown bracket type");
            else
            {
                sprintf(string, "%x,%p", current_argument->tag, current_argument->info.code);
                put_result = fputs(string, file);
            }
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
            {
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
                return;
            }
            current_argument = current_argument->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprints_0[] = {Z7 'F', 'P', 'R', 'I', 'N', 'T', 'S', (char)7};
G_L_B uint8_t refalab_fprints = '\122';
void (*fprints_1)(void) = fprints_;

static void fprintm_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag == TAGN)
        {
            const uint32_t file_number = gcoden(current_argument);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (current_argument->tag == TAGF)
        {
            if (current_argument->info.codef == &refalab_stdout)
                file = stdout;
            else if (current_argument->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        current_argument = current_argument->next;
        bool tago = false;
        while (current_argument != refal.next_argument)
        {
            int put_result = 0;
            char string[512];
            if (current_argument->tag == TAGO)
            {
                if (!tago)
                {
                    tago = true;
                    sprintf(string, "'%c", current_argument->info.infoc);
                    put_result = fputs(string, file);
                }
                else
                    put_result = putc(current_argument->info.infoc, file);
            }
            else
            {
                if (tago)
                {
                    tago = false;
                    sprintf(string, "'");
                }
                else
                    string[0] = '\0';
                if (current_argument->tag == TAGLB)
                    strcat(string, "(");
                else if (current_argument->tag == TAGRB)
                    strcat(string, ")");
                else if (current_argument->tag == TAGN)
                {
                    char string_number[512];
                    sprintf(string_number, "%u", gcoden(current_argument));
                    strcat(string, string_number);
                    if (current_argument->next->tag == TAGN)
                        strcat(string, " ");
                }
                else if (current_argument->tag == TAGF)
                {
                    strcat(string, "&");
                    const uint8_t *label_length = current_argument->info.codef - 1;
                    const char *label = (char *)label_length - *label_length;
                    char symbol[2];
                    symbol[1] = '\0';
                    for (uint8_t i = 1; i <= *label_length; i++, label++)
                    {
                        symbol[0] = (char)toupper(*label);
                        strcat(string, symbol);
                    }
                    if (current_argument->next->tag == TAGN)
                        strcat(string, " ");
                }
                else if (current_argument->tag == TAGR)
                {
                    char string_reference[512];
                    sprintf(string_reference, "/%%%p/", (void *)current_argument->info.codep);
                    strcat(string, string_reference);
                }
                else if (BRACKET(current_argument))
                    refal_abort_end("fprintm: unknown bracket type");
                else
                {
                    char string_symbol[512];
                    sprintf(string_symbol, "/%x,%p/", current_argument->tag, current_argument->info.code);
                    strcat(string, string_symbol);
                }
                put_result = fputs(string, file);
            }
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
            {
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
                return;
            }
            current_argument = current_argument->next;
        }
        if (tago)
        {
            const int put_result = putc('\'', file);
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprintm_0[] = {Z7 'F', 'P', 'R', 'I', 'N', 'T', 'M', (char)7};
G_L_B uint8_t refalab_fprintm = '\122';
void (*fprintm_1)(void) = fprintm_;

static void fread_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(current_argument);
        if (file_number >= FILES_MAX)
            break;
        file = files[file_number];
        current_argument = current_argument->next;
        if (current_argument->tag != TAGN)
            break;
        uint32_t symbols_count = gcoden(current_argument);
        if (current_argument->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        T_LINKCB *current_result = refal.previous_result;
        if (!extended_insert_from_free_memory_list(current_result, symbols_count))
            return;
        for (; symbols_count > 0; symbols_count--)
        {
            current_result = current_result->next;
            current_result->info.code = NULL;
            const int get_result = getc(file);
            if (set_eof_linkcb(get_result, file, current_result))
            {
                insert_to_free_memory_list(current_result, refal.next_result);
                return;
            }
            current_result->tag = TAGN;
            pcoden(current_result, (uint8_t)get_result);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fread_0[] = {Z5 'F', 'R', 'E', 'A', 'D', (char)5};
G_L_B uint8_t refalab_fread = '\122';
void (*fread_1)(void) = fread_;

static void fwrite_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(current_argument);
        if (file_number >= FILES_MAX)
            break;
        file = files[file_number];
        const T_LINKCB *temp_argument = current_argument->next;
        bool impossible = false;
        while (temp_argument != refal.next_argument)
        {
            if (temp_argument->tag != TAGO && temp_argument->tag != TAGN)
            {
                impossible = true;
                break;
            }
            temp_argument = temp_argument->next;
        }
        if (impossible)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        current_argument = current_argument->next;
        while (current_argument != refal.next_argument)
        {
            int symbol;
            if (current_argument->tag == TAGO)
                symbol = current_argument->info.infoc;
            else
                symbol = (uint8_t)gcoden(current_argument);
            const int put_result = putc(symbol, file);
            if (set_eof_linkcb(put_result, file, refal.previous_argument))
            {
                transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
                return;
            }
            current_argument = current_argument->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fwrite_0[] = {Z6 'F', 'W', 'R', 'I', 'T', 'E', (char)6};
G_L_B uint8_t refalab_fwrite = '\122';
void (*fwrite_1)(void) = fwrite_;

static void fseek_(void)
{
    do
    {
        T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(current_argument);
        if (file_number >= FILES_MAX)
            break;
        file = files[file_number];
        current_argument = current_argument->next;
        const char sign = current_argument->info.infoc;
        int64_t sign_digit = 1;
        if (current_argument->tag == TAGO && (sign == '-' || sign == '+'))
        {
            current_argument = current_argument->next;
            if (sign == '-')
                sign_digit = -1;
        }
        if (current_argument->tag != TAGN)
            break;
        const int64_t offset_absolute = gcoden(current_argument);
        if (sign_digit == 1 ? offset_absolute > 2147483647 : offset_absolute > 2147483648)
            break;
        const long int offset = (long int)(sign_digit * offset_absolute);
        current_argument = current_argument->next;
        if (current_argument->tag != TAGF)
            break;
        int origin;
        if (current_argument->info.codef == &refalab_begin)
            origin = SEEK_SET;
        else if (current_argument->info.codef == &refalab_end)
            origin = SEEK_END;
        else if (current_argument->info.codef == &refalab_cur)
            origin = SEEK_CUR;
        else
            break;
        if (current_argument->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        const int seek_result = fseek(file, offset, origin);
        if (seek_result == -1)
        {
            const int error_number = errno;
            const char *string_error = strerror(error_number);
            if (!extended_insert_from_free_memory_list(refal.next_result, strlen(string_error) - 3 - (sign_digit == 1 ? 1 : 2)))
                return;
            set_string_expression(string_error, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, refal.next_argument);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fseek_0[] = {Z5 'F', 'S', 'E', 'E', 'K', (char)5};
G_L_B uint8_t refalab_fseek = '\122';
void (*fseek_1)(void) = fseek_;

static void ftell_(void)
{
    do
    {
        T_LINKCB *symbol_number = refal.previous_argument->next;
        if (symbol_number->tag != TAGN)
            break;
        const uint32_t file_number = gcoden(symbol_number);
        if (file_number >= FILES_MAX)
            break;
        if (symbol_number->next != refal.next_argument)
            break;
        file = files[file_number];
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            return;
        }
        long int tell_result = ftell(file);
        if (tell_result == -1)
        {
            const int error_number = errno;
            const char *string_error = strerror(error_number);
            if (!extended_insert_from_free_memory_list(refal.next_result, strlen(string_error) - 2))
                return;
            set_string_expression(string_error, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, refal.next_argument);
            return;
        }
        pcoden(symbol_number, (uint32_t)tell_result);
        transplantation(refal.previous_result, symbol_number->previous, symbol_number->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char ftell_0[] = {Z5 'F', 'T', 'E', 'L', 'L', (char)5};
G_L_B uint8_t refalab_ftell = '\122';
void (*ftell_1)(void) = ftell_;

static void is_eof_(void)
{
    do
    {
        T_LINKCB *p = refal.previous_argument->next;
        if (p->tag == TAGN)
        {
            const uint32_t file_number = gcoden(p);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdin)
                file = stdin;
            else if (p->info.codef == &refalab_stdout)
                file = stdout;
            else if (p->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (p->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
            return;
        }
        if (feof(file) != 0 || ferror(file) != 0)
            refal.previous_argument->info.codef = &refalab_true;
        else
            refal.previous_argument->info.codef = &refalab_false;
        transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_eof_0[] = {Z6 'I', 'S', '_', 'E', 'O', 'F', (char)6};
G_L_B uint8_t refalab_is_eof = '\122';
void (*is_eof_1)(void) = is_eof_;

static void is_feof_(void)
{
    do
    {
        T_LINKCB *p = refal.previous_argument->next;
        if (p->tag == TAGN)
        {
            const uint32_t file_number = gcoden(p);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdin)
                file = stdin;
            else if (p->info.codef == &refalab_stdout)
                file = stdout;
            else if (p->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (p->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
            return;
        }
        if (feof(file) != 0)
            refal.previous_argument->info.codef = &refalab_true;
        else
            refal.previous_argument->info.codef = &refalab_false;
        transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_feof_0[] = {Z7 'I', 'S', '_', 'F', 'E', 'O', 'F', (char)7};
G_L_B uint8_t refalab_is_feof = '\122';
void (*is_feof_1)(void) = is_feof_;

static void is_ferror_(void)
{
    do
    {
        T_LINKCB *p = refal.previous_argument->next;
        if (p->tag == TAGN)
        {
            const uint32_t file_number = gcoden(p);
            if (file_number >= FILES_MAX)
                break;
            file = files[file_number];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdin)
                file = stdin;
            else if (p->info.codef == &refalab_stdout)
                file = stdout;
            else if (p->info.codef == &refalab_stderr)
                file = stderr;
            else
                break;
        }
        else
            break;
        if (p->next != refal.next_argument)
            break;
        if (file == NULL)
        {
            refal.previous_argument->info.codef = &refalab_null;
            transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
            return;
        }
        if (ferror(file) != 0)
            refal.previous_argument->info.codef = &refalab_true;
        else
            refal.previous_argument->info.codef = &refalab_false;
        transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_ferror_0[] = {Z1 'I', 'S', '_', 'F', 'E', 'R', 'R', 'O', 'R', (char)9};
G_L_B uint8_t refalab_is_ferror = '\122';
void (*is_ferror_1)(void) = is_ferror_;

static void remove_file_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    char file_name[MAX_PATHFILENAME + 1];
    p = get_string_expression(file_name, MAX_PATHFILENAME, p, refal.next_argument);
    if (p != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    const int u = unlink(file_name);
    if (u == -1)
    {
        const int error_number = errno;
        const char *string_error = strerror(error_number);
        const int32_t d = (int32_t)strlen(string_error) - ((int32_t)strlen(file_name) + 1);
        if (d > 0)
            if (!extended_insert_from_free_memory_list(refal.next_result, (size_t)d))
                return;
        p = set_string_expression(string_error, refal.next_result);
        transplantation(refal.previous_result, refal.next_result, p->next);
    }
    return;
}
char remove_file_0[] = {Z3 'R', 'E', 'M', 'O', 'V', 'E', '_', 'F', 'I', 'L', 'E', (char)11};
G_L_B uint8_t refalab_remove_file = '\122';
void (*remove_file_1)(void) = remove_file_;

static void rename_(void)
{
    do
    {
        T_LINKCB *p = refal.previous_argument->next;
        char namf[MAX_PATHFILENAME + 1];
        p = get_string_expression(namf, MAX_PATHFILENAME, p, refal.next_argument);
        if (p->tag != TAGN || gcoden(p) != 0)
            break;
        p = p->next;
        char namt[MAX_PATHFILENAME + 1];
        p = get_string_expression(namt, MAX_PATHFILENAME, p, refal.next_argument);
        if (p != refal.next_argument)
            break;
        const int r = rename(namf, namt);
        if (r == -1)
        {
            const int error_number = errno;
            const char *string_error = strerror(error_number);
            const int32_t d = (int32_t)strlen(string_error) - ((int32_t)strlen(namf) + (int32_t)strlen(namt) + 2);
            if (d > 0)
                if (!extended_insert_from_free_memory_list(refal.next_result, (size_t)d))
                    return;
            p = set_string_expression(string_error, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, p->next);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char rename_0[] = {Z6 'R', 'E', 'N', 'A', 'M', 'E', (char)6};
G_L_B uint8_t refalab_rename = '\122';
void (*rename_1)(void) = rename_;

static void exist_file_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    char file_name[MAX_PATHFILENAME + 1];
    p = get_string_expression(file_name, MAX_PATHFILENAME, p, refal.next_argument);
    if (p != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    struct stat st_buf;
    if (stat(file_name, &st_buf) == 0 && S_ISREG(st_buf.st_mode))
        refal.previous_argument->info.codef = &refalab_true;
    else
        refal.previous_argument->info.codef = &refalab_false;
    transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
    return;
}
char exist_file_0[] = {Z2 'E', 'X', 'I', 'S', 'T', '_', 'F', 'I', 'L', 'E', (char)10};
G_L_B uint8_t refalab_exist_file = '\122';
void (*exist_file_1)(void) = exist_file_;

static void exist_dir_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    char namd[MAX_PATHFILENAME + 1];
    p = get_string_expression(namd, MAX_PATHFILENAME, p, refal.next_argument);
    if (p != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    struct stat st_buf;
    if (stat(namd, &st_buf) == 0 && S_ISDIR(st_buf.st_mode))
        refal.previous_argument->info.codef = &refalab_true;
    else
        refal.previous_argument->info.codef = &refalab_false;
    transplantation(refal.previous_result, refal.next_result, refal.previous_argument->next);
    return;
}
char exist_dir_0[] = {Z1 'E', 'X', 'I', 'S', 'T', '_', 'D', 'I', 'R', (char)9};
G_L_B uint8_t refalab_exist_dir = '\122';
void (*exist_dir_1)(void) = exist_dir_;

//----------  end of file XFIO.C  -----------
