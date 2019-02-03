/*
 *  sqlite3xx - sqlite3 C++ layer, following the ideas of libpqxx
 *  Copyright (C) 2009  Andreas Baumann
 *
 *  This copyrighted material is made available to anyone wishing to use,
 *  modify, copy, or redistribute it subject to the terms and conditions of
 *  the GNU Lesser General Public License, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this distribution; if not, write to:
 *  Free Software Foundation, Inc.
 *  51 Franklin Street, Fifth Floor
 *  Boston, MA  02110-1301  USA
 *
 */

#ifndef __SQLITE3_PORT_STDINT_H
#define __SQLITE3_PORT_STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite3xx/port/sys.h"

/* for uintptr_t and snprintf placeholder PRIdPTR (FIXME:
 * needs a lot of improvement!) 
 */
#if defined SUNOS

#if OS_MAJOR_VERSION == 5

#if OS_MINOR_VERSION == 8
#include <sys/int_types.h>
/* FIXME: this may depend on 32/64-bit version of Solaris 8! */
#define PRIdPTR "d"
typedef unsigned long long uint64_t;
#else
#include <inttypes.h>
#include <stdint.h>
#endif /* OS_MINOR_VERSION == 8 */
#else
#include <inttypes.h>
#include <stdint.h>
#endif /* OS_MAJOR_VERSION == 5 */
#else

#if !defined _WIN32
#include <inttypes.h>
#include <stdint.h>
#else
#include <windows.h>
#define uintmax_t __int64
#endif /* !defined _WIN32 */

#endif /* defined SUNOS */

#ifdef __cplusplus
}
#endif

#endif /* ifndef __SQLITE3_PORT_STDINT_H */
