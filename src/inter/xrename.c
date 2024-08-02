//---------- file -- XRENAME.C ------------
//           MO: file delete/rename
//      Last edition date : 11.07.24
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "refal.def"
#include "rfintf.h"

static void unlnk_()
{
    const T_LINKCB *p = refal.preva->next;
    size_t i;
    char namf[40];
    for (i = 0; p != refal.nexta; i++)
        if ((p->tag != TAGO) || (i >= 40))
        {
            printf("\nUnlnk: format error");
            refal.upshot = 2;
            return;
        }
        else
        {
            namf[i] = p->info.infoc;
            p = p->next;
        }
    namf[i] = '\0';
    //   printf("\nUnlnk, namf=%s",namf);
    if (unlink(namf) == -1)
    {
        printf("\nUnlnk: error");
        exit(1);
    }
    return;
}
static char unlnk_0[] = {Z5 'U', 'N', 'L', 'N', 'K', '\005'};
G_L_B char unlnk = '\122';
static void (*unlnk_1)() = unlnk_;

static void renam_()
{
    const T_LINKCB *p = refal.preva->next;
    size_t i;
    char namf[40];
    bool heot = false;
    do
    {
        for (i = 0; p->tag != TAGO || p->info.infoc != ' '; i++)
            if ((p->tag != TAGO) || (i >= 40))
            {
                heot = true;
                break;
            }
            else
            {
                namf[i] = p->info.infoc;
                p = p->next;
            }
        if (heot)
            break;
        namf[i] = '\0';
        p = p->next;
        char namt[40]; // from => to
        for (i = 0; p != refal.nexta; i++)
            if ((p->tag != TAGO) || (i >= 40))
            {
                heot = true;
                break;
            }
            else
            {
                namt[i] = p->info.infoc;
                p = p->next;
            }
        if (heot)
            break;
        namt[i] = '\0';
        //   printf("\nRenam: from=%s to=%s",namf,namt);
        if (rename(namf, namt) == -1)
        {
            printf("\nRenam: error");
            exit(1);
        }
        return;
    } while (false);
    printf("\nRenam: format error");
    refal.upshot = 2;
    return;
}
static char renam_0[] = {Z5 'R', 'E', 'N', 'A', 'M', '\005'};
G_L_B char renam = '\122';
static void (*renam_1)() = renam_;

//-------- end of file  XRENAME.C -------
