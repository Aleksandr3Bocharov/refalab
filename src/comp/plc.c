//--------------------  file  --  PLC.C --------------------
//     standard PL/I-functions implemented on C-language
//                Last edition date : 11.07.24
//----------------------------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include "plc.h"

int32_t index_x(const char *s, const char *t)
// first left insertion t into  s
// if not: -1
{
    const size_t imax = 40;
    size_t l = 0;
    for (size_t i = 0; i < imax; i++)
        if (*(s + i) == '\0')
            break;
        else
            l = i + 1;
    // l - length of s

    size_t m;
    for (size_t i = 0; i < imax; i++)
        if (*(t + i) == '\0')
            break;
        else
            m = i + 1;
    // m - length of t

    for (size_t i = 0; (i < l) && (*(s + i) != '\0'); i++)
        for (size_t j = i, k = 0; (j < l) && (*(s + j) != '\0'); j++)
        {
            if (*(s + j) != *(t + k))
                break;
            k++;
            if (k >= m)
                return i;
        }
    return -1;
}

// int index(char *s, int l, char *t, int m)
// first left insertion t into  s
// if not: -1
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

/*
int32_t cmpstr(size_t n, const char *s1, const char *s2)
// comparison two string . if s1<s2 then return < 0
// if s1 = s2 return 0. if s1>s2 then return > 0
{
    for (size_t i = 0; i < n; i++)
        if (*(s1 + i) != *(s2 + i))
            return *(s1 + i) - *(s2 + i);
    return 0;
} */
//----------  end of file PLC.C  -----------
