/*
 * common.c
 * contains some common functions
 *
 * Copyright (c) 2026 Nikias Bassen, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"

size_t dtostr(char *buf, size_t bufsize, double realval)
{
    int slen = 0;
    if (isnan(realval)) {
        slen = snprintf(buf, bufsize, "nan");
    } else if (isinf(realval)) {
        slen = snprintf(buf, bufsize, "%cinfinity", (realval > 0.0) ? '+' : '-');
    } else if (realval == 0.0) {
        slen = snprintf(buf, bufsize, "%s0.0", signbit(realval) ? "-" : "");
    } else {
        slen = snprintf(buf, bufsize, "%.*g", 17, realval);
        if (slen < 0) {
            return 0;
        }
        if (!buf || bufsize == 0) {
            return (size_t)slen;
        }
        size_t len = (size_t)slen;
        if (len >= bufsize) {
            len = bufsize - 1;
        }
        size_t i = 0;
        for (i = 0; i < len; i++) {
            if (buf[i] == ',') {
                buf[i] = '.';
                break;
            } else if (buf[i] == '.') {
                break;
            }
        }
        return len;
    }
    if (slen < 0) {
        return 0;
    }
    return (size_t)slen;
}

/* based on https://stackoverflow.com/a/4143288 */
#define PO10i_LIMIT (INT64_MAX/10)
int num_digits_i(int64_t i)
{
    int n;
    int64_t po10;
    n=1;
    if (i < 0) {
        i = (i == INT64_MIN) ? INT64_MAX : -i;
        n++;
    }
    po10=10;
    while (i>=po10) {
        n++;
        if (po10 > PO10i_LIMIT) break;
        po10*=10;
    }
    return n;
}
#undef PO10i_LIMIT

/* based on https://stackoverflow.com/a/4143288 */
#define PO10u_LIMIT (UINT64_MAX/10)
int num_digits_u(uint64_t i)
{
    int n;
    uint64_t po10;
    n=1;
    po10=10;
    while (i>=po10) {
        n++;
        if (po10 > PO10u_LIMIT) break;
        po10*=10;
    }
    return n;
}
#undef PO10u_LIMIT

int plist_real_to_time64(double realval, Time64_T *timev)
{
    if (!timev || !isfinite(realval)) {
        return -1;
    }

    if (realval < (double)TIME64_MIN - (double)MAC_EPOCH ||
        realval > (double)TIME64_MAX - (double)MAC_EPOCH) {
        return -1;
    }

    *timev = (Time64_T)realval + MAC_EPOCH;
    return 0;
}
