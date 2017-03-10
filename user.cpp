/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "amsd.hpp"

string amsd_local_superuser_token = amsd_random_string();


int amsd_user_login(string user, string passwd, string &token){

	sqlite3 *db;
	sqlite3_stmt *stmt;
	int ret = 0;

	db_open(dbpath_user, db);

	sqlite3_prepare(db, "UPDATE user SET Token = ?1 WHERE UserName = ?2 AND Password = ?3", -1, &stmt, NULL);

	token = amsd_random_string();

	u_char hashed_passwd[64];

	SHA512((const u_char *)passwd.c_str(), passwd.length(), hashed_passwd);

	sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, user.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_blob(stmt, 3, hashed_passwd, 64, SQLITE_STATIC);

	sqlite3_step(stmt);

	if (!sqlite3_changes(db))
		ret = 2;

	sqlite3_finalize(stmt);
	db_close(db);

	return ret;
}

int amsd_user_auth(string token, User *userinfo){
	if (token == amsd_local_superuser_token)
		return 0;

	sqlite3 *db;
	sqlite3_stmt *stmt;
	int ret = 0;

	db_open(dbpath_user, db);

	sqlite3_prepare(db, "SELECT * FROM user WHERE Token = ?1", -1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		if (userinfo) {

		}
	} else {
		ret = 1;
	}

	sqlite3_finalize(stmt);
	db_close(db);

	return ret;
}