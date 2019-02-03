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

#ifndef SQLITE3XX_TRANSACTION_H
#define SQLITE3XX_TRANSACTION_H

#include "sqlite3xx/connection.hpp"
#include "sqlite3xx/result.hpp"
#include "sqlite3xx/prepared_statement.hpp"
#include "sqlite3.h"

#include "port/dllexport.h"

namespace sqlite3xx {

class SQLITEXX_LIBEXPORT transaction {
	private:
		enum Status {
			st_nascent,
			st_active,
			st_aborted,
			st_committed
		};

	private:
		connection& _c;
		string _name;
		Status _status;

	protected:
		void Begin( );

	public:
		transaction( connection &c, string name = 0 );
		~transaction( );

		void commit( );
		void abort( );

		result exec( string sql );
		prepare::invocation prepared( const string& name );
};

typedef transaction work;

}

#endif /* SQLITE3XX_TRANSACTION_H */
