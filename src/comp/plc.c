// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-15
// https://github.com/Aleksandr3Bocharov/RefalAB

//--------------------  file  --  PLC.C --------------------
//     standard PL/I-functions implemented on C-language
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

    size_t m = 0;
    for (size_t i = 0; i < imax; i++)
        if (*(t + i) == '\0')
            break;
        else
            m = i + 1;
    // m - length of t

    for (size_t i = 0; i < l && *(s + i) != '\0'; i++)
        for (size_t j = i, k = 0; j < l && *(s + j) != '\0'; j++)
        {
            if (*(s + j) != *(t + k))
                break;
            k++;
            if (k >= m)
                return (int32_t)i;
        }
    return -1;
}

//----------  end of file PLC.C  -----------
