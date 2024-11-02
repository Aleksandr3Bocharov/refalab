// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-11-02
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XVV5.C ------------
//    REFAL-5 compatible file input/output
//------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "refal.def"
#include "rfintf.h"

#define fmax 5

static FILE *f;
static FILE *uniput[fmax] = {NULL, NULL, NULL, NULL, NULL};
static FILE *uniget[fmax] = {NULL, NULL, NULL, NULL, NULL};

static void open_(void)
{
   char namf[41];
   for (size_t i = 0; i < 40; i++)
      namf[i] = '\0';
   const T_LINKCB *p = refal.preva->next;
   bool neot1 = true;
   do
   {
      if (p->tag != TAGO)
         break;
      const char c = p->info.infoc;
      p = p->next;
      uint32_t j;
      if (p->tag != TAGN)
         break;
      j = gcoden(p);
      p = p->next;
      if (j >= fmax)
         break;
      char s[2];
      s[1] = 0;
      if (c == 'R' || c == 'r')
         s[0] = 'r';
      else if (c == 'W' || c == 'w')
         s[0] = 'w';
      else
         break;
      bool neot = false;
      for (size_t i = 0; p != refal.nexta; i++)
      {
         if (p->tag != TAGO || i == 40)
         {
            neot = true;
            break;
         }
         namf[i] = p->info.infoc;
         p = p->next;
      }
      if (neot)
         break;
      f = fopen(namf, s);
      if (f == NULL)
      {
         printf("\n xopen: can't open file %s", namf);
         neot1 = true;
         break;
      }
      if (s[0] == 'r')
         uniget[j] = f;
      else
         uniput[j] = f;
      return;
   } while (false);
   if (!neot1)
      printf("\n xopen: format error");
   refal.upshot = 2;
   return;
}
char open_0[] = {Z5 'X', 'O', 'P', 'E', 'N', '\005'};
G_L_B char xopen = '\122';
void (*open_1)(void) = open_;

static void close_(void)
{
   const T_LINKCB *p = refal.preva->next;
   do
   {
      if (p->tag != TAGO)
         break;
      const char c = p->info.infoc;
      p = p->next;
      if (p->tag != TAGN)
         break;
      const uint32_t j = gcoden(p);
      if (j >= fmax)
         break;
      if (c == 'R' || c == 'r')
      {
         f = uniget[j];
         uniget[j] = NULL;
      }
      else if (c == 'W' || c == 'w')
      {
         f = uniput[j];
         uniput[j] = NULL;
      }
      else
         break;
      fclose(f);
      return;
   } while (false);
   printf("\nxclose: format error");
   refal.upshot = 2;
   return;
}
char close_0[] = {Z6 'X', 'C', 'L', 'O', 'S', 'E', '\006'};
G_L_B char xclose = '\122';
void (*close_1)(void) = close_;

static void get_(void)
{
   T_LINKCB *p = refal.preva->next;
   bool neot1 = false;
   do
   {
      if (p->tag != TAGN)
         break;
      const uint32_t j = gcoden(p);
      if (j >= fmax)
         break;
      f = uniget[j];
      if (f == NULL)
      {
         char namf[11];
         strcpy(namf, "REFAL0.DAT");
         namf[5] = (char)j + '0';
         f = fopen(namf, "r");
         if (f == NULL)
         {
            printf("\nxget: can't open file %s", namf);
            neot1 = true;
            break;
         }
         uniget[j] = f;
      }
      p = refal.prevr;
      int c = getc(f);
      while (c != '\n')
      {
         if (!slins(p, 1))
            return;
         p = p->next;
         p->info.codep = NULL;
         if (c == EOF || c == '\032')
         {
            p->tag = TAGN;
            return;
         }
         p->tag = TAGO;
         p->info.infoc = (char)c;
         c = getc(f);
      }
      return;
   } while (false);
   if (!neot1)
      printf("\nxget: format error");
   refal.upshot = 2;
   return;
}
char get_0[] = {Z4 'X', 'G', 'E', 'T', '\004'};
G_L_B char xget = '\122';
void (*get_1)(void) = get_;

static void put_(void)
{
   const T_LINKCB *p = refal.preva->next;
   bool neot1 = false;
   do
   {
      if (p->tag != TAGN)
         break;
      const uint32_t j = gcoden(p);
      p = p->next;
      if (j >= fmax)
         break;
      f = uniput[j];
      if (f == NULL)
      {
         char namf[11];
         strcpy(namf, "REFAL0.DAT");
         namf[5] = (char)j + '0';
         f = fopen(namf, "w");
         if (f == NULL)
         {
            printf("\nxput: can't open file %s", namf);
            neot1 = true;
            break;
         }
         uniput[j] = f;
      }
      while (p != refal.nexta)
      {
         int cc;
         if (p->tag != TAGO)
         {
            if (p->tag != TAGLB && p->tag != TAGRB)
            {
               refal.upshot = 2;
               return;
            }
            else
            {
               if (p->tag == TAGLB)
                  cc = '(';
               else
                  cc = ')';
               putc(cc, f);
               p = p->next;
            }
         }
         else
         {
            cc = p->info.infoc;
            putc(cc, f);
            p = p->next;
         }
      }
      putc('\n', f);
      return;
   } while (false);
   if (!neot1)
      printf("\nxput: format error");
   refal.upshot = 2;
   return;
}
char put_0[] = {Z4 'X', 'P', 'U', 'T', '\004'};
G_L_B char xput = '\122';
void (*put_1)(void) = put_;

//----------  end of file XVV5.C  -----------
