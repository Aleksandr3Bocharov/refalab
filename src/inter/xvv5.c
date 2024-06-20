/*-----------  file  --  XVV5.C ------------*/
/*    REFAL-5 compatible file input/output  */
/*       Last modification : 19.06.24       */
/*------------------------------------------*/
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
   linkcb *p;
   short j;
   char namf[41], c, s[2];
   register int i;
   for (i = 0; i < 40; i++)
      namf[i] = '\0';
   p = refal.preva->next;
   if (p->tag != TAGO)
      goto NEOT;
   c = p->info.infoc;
   p = p->next;
   if (p->tag != TAGN)
      goto NEOT;
   else
   {
      j = (short)p->info.coden;
      p = p->next;
   }
   if (j >= fmax)
      goto NEOT;
   s[1] = 0;
   if (c == 'R' || c == 'r')
      s[0] = 'r';
   else if (c == 'W' || c == 'w')
      s[0] = 'w';
   else
      goto NEOT;
   for (i = 0; p != refal.nexta; i++)
      if ((p->tag != TAGO) || (i == 40))
         goto NEOT;
      else
      {
         namf[i] = p->info.infoc;
         p = p->next;
      }
   if ((f = fopen(namf, s)) == NULL)
   {
      printf("\n REFAL(open): can't open file %s", namf);
      goto NEOT1;
   }
   if (s[0] == 'r')
      uniget[j] = f;
   else
      uniput[j] = f;
   return;
NEOT:
   printf("\n REFAL(open): format error");
NEOT1:
   refal.upshot = 2;
   return;
}
static char open_0[] = {Z4 'O', 'P', 'E', 'N', '\004'};
G_L_B char xopen = '\122';
static void (*open_1)() = open_;

static void close_()
{
   linkcb *p;
   char c;
   short j;
   p = refal.preva->next;
   if (p->tag != TAGO)
      goto NEOT;
   c = p->info.infoc;
   p = p->next;
   if (p->tag != TAGN)
      goto NEOT;
   j = (short)p->info.coden;
   if (j >= fmax)
      goto NEOT;
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
      goto NEOT;
   /*printf("\n close, flg=%d fd=%d",f->flags,f->fd);*/
   fclose(f);
   return;
NEOT:
   printf("\nREFAL(close): format error");
   refal.upshot = 2;
   return;
}
static char close_0[] = {Z5 'C', 'L', 'O', 'S', 'E', '\005'};
G_L_B char xclose = '\122';
static void (*close_1)() = close_;

static void get_()
{
   linkcb *p;
   short i, j;
   char c, namf[11];
   p = refal.preva->next;
   if (p->tag != TAGN)
      goto NEOT;
   j = (short)p->info.coden;
   if (j >= fmax)
      goto NEOT;
   f = uniget[j];
   if (f == NULL)
   {
      strcpy(namf, "REFAL0.DAT");
      namf[5] = j + '0';
      if ((f = fopen(namf, "r")) == NULL)
      {
         printf("\nREFAL(get): can't open file %s", namf);
         goto NEOT1;
      }
      uniget[j] = f;
   }
   p = refal.prevr;
   while ((c = getc(f)) != '\n')
   {
      if (slins(p, 1) == 0)
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
NEOT:
   printf("\nREFAL(get): format error");
NEOT1:
   refal.upshot = 2;
   return;
}
static char get_0[] = {Z3 'G', 'E', 'T', '\003'};
G_L_B char xget = '\122';
static void (*get_1)() = get_;

static void put_()
{
   linkcb *p;
   short j;
   int i, cc;
   char namf[11];
   p = refal.preva->next;
   if (p->tag != TAGN)
      goto NEOT;
   j = (short)p->info.coden;
   p = p->next;
   if (j >= fmax)
      goto NEOT;
   f = uniput[j];
   if (f == NULL)
   {
      strcpy(namf, "REFAL0.DAT");
      namf[5] = j + '0';
      if ((f = fopen(namf, "w")) == NULL)
      {
         printf("\nREFAL(put): can't open file %s", namf);
         goto NEOT1;
      }
      uniput[j] = f;
   }
   while (p != refal.nexta)
   {
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
NEOT:
   printf("\nREFAL(put): format error");
NEOT1:
   refal.upshot = 2;
   return;
}
static char put_0[] = {Z3 'P', 'U', 'T', '\003'};
G_L_B char xput = '\122';
static void (*put_1)() = put_;

/*----------  end of file XVV5.C  -----------*/
