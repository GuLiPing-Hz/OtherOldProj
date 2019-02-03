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

#ifndef SQLITE3XX_EXCEPT_H
#define SQLITE3XX_EXCEPT_H

#include <stdexcept>

#include "port/dllexport.h"

using namespace std;

namespace sqlite3xx {

class SQLITEXX_LIBEXPORT sql_error : runtime_error {
	string _m;
	string _q;

	public:
		sql_error( );
		explicit sql_error( string& q );
		explicit sql_error( string& __m, string& q );
		virtual ~sql_error( ) throw( );

		const string& msg( ) const throw( );
		const string& query( ) const throw( );
};

}

#endif /* SQLITE3XX_EXCEPT_H */
