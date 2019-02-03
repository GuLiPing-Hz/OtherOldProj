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

#ifndef SQLITE3XX_PREPARED_STATEMENT_H
#define SQLITE3XX_PREPARED_STATEMENT_H

#include <vector>
#include <string>

#include <sqlite3xx/port/stdint.h>		/* for uintmax_t */

#include "port/dllexport.h"

#include "sqlite3.h"

using namespace std;

namespace sqlite3xx {

class connection;
class transaction;
class result;

namespace prepare {

	enum param_treatment {
		treat_direct
	};

	/* class to add () parameters to a prepare statement */
	class SQLITEXX_LIBEXPORT declaration {
		private:
			connection& _c;
			const string& _stmt;

		public:
			declaration( connection &c, const string& stmt );
		
			const declaration& operator( )( const string& sql_type, 
				param_treatment treat ) const;
	};

	class SQLITEXX_LIBEXPORT invocation {
		private:
			connection& _c;
			transaction& _t;
			const string& _stmt;
			int _pos;

		public:
			invocation( connection& c, transaction& t, const string& stmt );

			invocation &setparam( const int& value, bool nonnull );
			invocation &setparam( const char* value, bool nonnull );
			invocation &setparam( const string& value, bool nonnull );
			invocation &setparam( const double& value, bool nonnull );
			invocation &setparam( const long& value, bool nonnull );
			invocation &setparam( const uintmax_t& value, bool nonnull );

			result exec( ) const;

			template<typename T>
				invocation &operator( )( const T &v, bool nonnull = true ) {
					return setparam( v, nonnull );
			}
	};
}

class SQLITEXX_LIBEXPORT prepared_stmt {

	private:
		typedef pair<string,prepare::param_treatment> param;

		sqlite3_stmt *_stmt;
		string _sql;
		vector<param> parameters;
		sqlite3 *_db;

	public:
		prepared_stmt( sqlite3 *db, string __sql );
		~prepared_stmt( );
		string sql( );
		void addparam( const string& sqltype, prepare::param_treatment& treatment );

		void reset( );
		void setparam( const int pos, const int value );
		void setparam( const int pos, const char* value );
		void setparam( const int pos, const double value );
		void setparam( const int pos, const long value );
		void setparam( const int pos, const uintmax_t value );

		sqlite3_stmt* getStmt( ) const;
};

} /* namespace sqlite3xx */

#endif /* SQLITE3XX_PREPARED_STATEMENT_H */
