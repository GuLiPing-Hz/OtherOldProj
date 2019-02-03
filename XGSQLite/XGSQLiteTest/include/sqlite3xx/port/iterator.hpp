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
 *  MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this distribution; if not, write to:
 *  Free Software Foundation, Inc.
 *  51 Franklin Street, Fifth Floor
 *  Boston, MA  02110-1301  USA
 *
 */

#ifndef SQLITE3XX_ITERATOR_H
#define SQLITE3XX_ITERATOR_H

#include "sqlite3xx/port/sys.h"

#ifdef SQLITE3XX_BROKEN_ITERATOR

#include <iterator>

namespace sqlite3xxstd {

template<typename Cat,
	typename T,
	typename Dist,
	typename Ptr=T*,
	typename Ref=T&> struct iterator {

	typedef Cat iterator_category;
	typedef T value_type;
	typedef Dist difference_type;
	typedef Ptr pointer;
	typedef Ref reference;
};

} /* namespace sqlite3xxstd */

#endif /* SQLITE3XX_BROKEN_ITERATOR */

#endif /* SQLITE3XX_ITERATOR_H */
