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

#ifndef __PORT_DLLEXPORT_H
#define __PORT_DLLEXPORT_H

#ifdef _WIN32
#ifdef BUILD_SHARED
#define SQLITEXX_LIBEXPORT __declspec( dllexport )
#define SQLITEXX_PRIVATE __declspec( )
#else
#define SQLITEXX_LIBEXPORT
#define SQLITEXX_PRIVATE
#endif
#else
#define SQLITEXX_LIBEXPORT
#define SQLITEXX_PRIVATE
#endif

#endif /* ifndef __PORT_DLLEXPORT_H */
