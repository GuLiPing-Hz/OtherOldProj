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

#ifndef SQLITE3XX_CONNECTION_H
#define SQLITE3XX_CONNECTION_H

#include "prepared_statement.hpp"
#include "result.hpp"

#include "sqlite3.h"

#include <map>
#include <iostream>

#include <sqlite3xx/port/stdint.h>		/* for uintmax_t */

#include "port/dllexport.h"

using namespace std;

namespace sqlite3xx {

class prepared_stmt;
class result;

class SQLITEXX_LIBEXPORT connection {
	private:
		typedef map<string, prepared_stmt *> PSMap;

		unsigned int _internal_tno;
		string _filename;
		sqlite3 *db;
		PSMap prepared_stmts;
		bool _trace;

		SQLITEXX_PRIVATE prepared_stmt* find_prepared( const string& name );

	public:
		connection( );
		connection( string filename );
		connection( const connection& c );
		~connection( );

		void trace( bool __trace );

		result exec( string sql );
		result prepared_exec( const string& name );

		prepare::declaration prepare( const string& name, string sql );
		void prepare_param_declare( const string& stmt, const string& sqltype, prepare::param_treatment& treatment );
		void prepared_reset( const string& name );
		void prepare_setparam( const string& name, const int pos, const int value );
		void prepare_setparam( const string& name, const int pos, const char* value );
		void prepare_setparam( const string& name, const int pos, const double value );
		void prepare_setparam( const string& name, const int pos, const long value );
		void prepare_setparam( const string& name, const int pos, const uintmax_t value );

		void unprepare( string name );

		SQLITEXX_LIBEXPORT friend ostream& operator<<( ostream& o, const connection& c );

		const char *dbname( ) { return _filename.c_str( ); };
};

}

#endif /* SQLITE3XX_CONNECTION_H */
