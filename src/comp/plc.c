/*--------------------  file  --  PLC.C --------------------*/
/*     standard PL/I-functions implemented on C-language    */
/*                Last edition date : 11.07.24              */
/*----------------------------------------------------------*/

#include "plc.h"

int index_x(const char *s, const char *t)
/* first left insertion t into  s */
/* if not: -1                     */
{
    const unsigned int imax = 40;
    unsigned int l = 0;
    for (unsigned int i = 0; i < imax; i++)
    {
        if (*(s + i) == '\0')
            break;
        else
            l = i + 1;
    } /* l - length of s */

    unsigned int m;
    for (unsigned int i = 0; i < imax; i++)
    {
        if (*(t + i) == '\0')
            break;
        else
            m = i + 1;
    } /* m - length of t */

    for (unsigned int i = 0; (i < l) && (*(s + i) != '\0'); i++)
        for (unsigned int j = i, k = 0; (j < l) && (*(s + j) != '\0'); j++)
        {
            if (*(s + j) != *(t + k))
                break;
            k++;
            if (k >= m)
                return i;
        }
    return -1;
}

/* int index(char *s, int l, char *t, int m) */
/* first left insertion t into  s */
/* if not: -1                     */
/*
{
  int i, j, k;
  for (i = 0; (i < l) && (*(s+i) != '\0') ; i++)
    for ( j=i, k=0; (j < l) && (*(s+j) != '\0'); j++ ){
       if (*(s+j) != *(t+k)) break;
       k++;
       if ( k >= m ) return i;
    }
  return -1;
}
*/

int cmpstr(unsigned int n, const char *s1, const char *s2)
/* comparison two string . if s1<s2 then return < 0 */
/* if s1 = s2 return 0. if s1>s2 then return > 0    */
{
    for (unsigned int i = 0; i < n; i++)
        if (*(s1 + i) != *(s2 + i))
            return (*(s1 + i) - *(s2 + i));
    return 0;
}
/*----------  end of file PLC.C  -----------*/
