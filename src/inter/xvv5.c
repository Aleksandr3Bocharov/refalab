//-----------  file  --  XVV5.C ------------
//    REFAL-5 compatible file input/output
//       Last modification : 11.07.24
//------------------------------------------
#include <stdio.h>
#include <string.h>
#include "refal.def"
#include "rfintf.h"

#define fmax 5

static FILE *f;
static FILE *uniput[fmax] = {NULL, NULL, NULL, NULL, NULL};
static FILE *uniget[fmax] = {NULL, NULL, NULL, NULL, NULL};

static void open_()
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
      j = p->info.coden;
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
         if ((p->tag != TAGO) || (i == 40))
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
         printf("\n REFAL(open): can't open file %s", namf);
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
      printf("\n REFAL(open): format error");
   refal.upshot = 2;
   return;
}
static char open_0[] = {Z4 'O', 'P', 'E', 'N', '\004'};
G_L_B char xopen = '\122';
static void (*open_1)() = open_;

static void close_()
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
      const uint32_t j = p->info.coden;
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
      // printf("\n close, flg=%d fd=%d",f->flags,f->fd);
      fclose(f);
      return;
   } while (false);
   printf("\nREFAL(close): format error");
   refal.upshot = 2;
   return;
}
static char close_0[] = {Z5 'C', 'L', 'O', 'S', 'E', '\005'};
G_L_B char xclose = '\122';
static void (*close_1)() = close_;

static void get_()
{
   T_LINKCB *p = refal.preva->next;
   bool neot1 = false;
   do
   {
      if (p->tag != TAGN)
         break;
      const uint32_t j = p->info.coden;
      if (j >= fmax)
         break;
      f = uniget[j];
      if (f == NULL)
      {
         char namf[11];
         strcpy(namf, "REFAL0.DAT");
         namf[5] = j + '0';
         f = fopen(namf, "r");
         if (f  == NULL)
         {
            printf("\nREFAL(get): can't open file %s", namf);
            neot1 = true;
            break;
         }
         uniget[j] = f;
      }
      p = refal.prevr;
      char c;
      while ((c = getc(f)) != '\n')
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
         p->info.infoc = c;
      }
      return;
   } while (false);
   if (!neot1)
      printf("\nREFAL(get): format error");
   refal.upshot = 2;
   return;
}
static char get_0[] = {Z3 'G', 'E', 'T', '\003'};
G_L_B char xget = '\122';
static void (*get_1)() = get_;

static void put_()
{
   const T_LINKCB *p = refal.preva->next;
   bool neot1 = false;
   do
   {
      if (p->tag != TAGN)
         break;
      const uint32_t j = p->info.coden;
      p = p->next;
      if (j >= fmax)
         break;
      f = uniput[j];
      if (f == NULL)
      {
         char namf[11];
         strcpy(namf, "REFAL0.DAT");
         namf[5] = j + '0';
         f = fopen(namf, "w");
         if (f == NULL)
         {
            printf("\nREFAL(put): can't open file %s", namf);
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
            if ((p->tag != TAGLB) && (p->tag != TAGRB))
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
      printf("\nREFAL(put): format error");
   refal.upshot = 2;
   return;
}
static char put_0[] = {Z3 'P', 'U', 'T', '\003'};
G_L_B char xput = '\122';
static void (*put_1)() = put_;

//----------  end of file XVV5.C  -----------
