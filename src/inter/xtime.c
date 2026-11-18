// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xtime.c  ---------
//        MO: Time (date and time),
//            Tm (timer),
//            Tm_elapsed (current interpretator time)
//-----------------------------------

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>
#include "refalab.h"
#include "interface.h"

// Symbol of the empty function NULL, returned by Time when the local time is unknown.
extern uint8_t refalab_null;

// Start and current moments of the stopwatch Tm and the moment for Tm_elapsed.
static T_TIMESPEC time_start, time_current, time_elapsed;

// Time: <Time> - the current date and time as a string of characters in the format of
// the system locale (strftime "%c"). The call must have no argument (FAIL otherwise).
// NULL is returned when the local time cannot be obtained.
static void time_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    setlocale(LC_TIME, "");
    const time_t current_time = time(NULL);
    const struct tm *current_local_time = localtime(&current_time);
    if (current_local_time == NULL)
    {
        refal.previous_argument->info.codef = &refalab_null;
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
        return;
    }
    char string_time[256];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-y2k"
    const size_t string_time_length = strftime(string_time, sizeof(string_time), "%c", current_local_time);
#pragma GCC diagnostic pop
    if (string_time_length == 0)
        return;
    if (!extended_insert_from_free_memory(refal.next_result, string_time_length - 1))
        return;
    set_string_expression(string_time, refal.next_result);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char time_0[] = {Z4 'T', 'I', 'M', 'E', (char)4};
G_L_B uint8_t refalab_time = '\122';
void (*time_1)(void) = time_;

// Tm: <Tm s.T> - a stopwatch. 'S' or 's' starts it (the result is empty), 'G' or 'g'
// returns the elapsed time as the string "HH:MM:SS.nnnnnnnnn"; without a start the time is
// counted from the zero start moment (the clock's own origin). FAIL for any other argument.
// The number of cells that are requested accounts for the cell of the argument,
// which is reused in the result.
static void tm_(void)
{
    do
    {
        T_LINKCB *symbol_char = refal.previous_argument->next;
        if (symbol_char->next != refal.next_argument || symbol_char->tag != TAGO)
            break;
        const char timer_command = symbol_char->info.infoc;
        if (timer_command == 'S' || timer_command == 's')
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        else if (timer_command == 'G' || timer_command == 'g')
        {
            clock_gettime(CLOCK_MONOTONIC, &time_current);
            // Elapsed time: seconds and nanoseconds are subtracted with a borrow,
            // then split into hours, minutes and seconds.
            long int nanoseconds = time_current.tv_nsec - time_start.tv_nsec;
            long long int seconds = (long long int)difftime(time_current.tv_sec, time_start.tv_sec);
            if (nanoseconds < 0)
            {
                nanoseconds += 1000000000;
                seconds--;
            }
            long long int minutes = seconds / 60;
            seconds %= 60;
            const long long int hours = minutes / 60;
            minutes %= 60;
            char string_time[64];
            sprintf(string_time, "%02lld:%02lld:%02lld.%09ld", hours, minutes, seconds, nanoseconds);
            if (!extended_insert_from_free_memory(refal.next_result, strlen(string_time) - 2))
                return;
            set_string_expression(string_time, refal.next_result);
            transplantation(refal.previous_result, refal.next_result, refal.next_argument);
        }
        else
            break;
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char tm_0[] = {Z2 'T', 'M', (char)2};
G_L_B uint8_t refalab_tm = '\122';
void (*tm_1)(void) = tm_;

// Tm_elapsed: <Tm_elapsed> - the time that has passed since the interpreter started, in the
// format of Tm. The result is empty unless the timer option is on (refal.timer.mode);
// the call must have no argument (FAIL otherwise).
static void tm_elapsed_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    if (!refal.timer.mode)
        return;
    clock_gettime(CLOCK_MONOTONIC, &time_elapsed);
    long int nanoseconds = time_elapsed.tv_nsec - refal.timer.start_time.tv_nsec;
    long long int seconds = (long long int)difftime(time_elapsed.tv_sec, refal.timer.start_time.tv_sec);
    if (nanoseconds < 0)
    {
        nanoseconds += 1000000000;
        seconds--;
    }
    long long int minutes = seconds / 60;
    seconds %= 60;
    const long long int hours = minutes / 60;
    minutes %= 60;
    char string_time[64];
    sprintf(string_time, "%02lld:%02lld:%02lld.%09ld", hours, minutes, seconds, nanoseconds);
    if (!extended_insert_from_free_memory(refal.next_result, strlen(string_time) - 1))
        return;
    set_string_expression(string_time, refal.next_result);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
}
char tm_elapsed_0[] = {Z2 'T', 'M', '_', 'E', 'L', 'A', 'P', 'S', 'E', 'D', (char)10};
G_L_B uint8_t refalab_tm_elapsed = '\122';
void (*tm_elapsed_1)(void) = tm_elapsed_;

//----------- end of file xtime.c  ---------
