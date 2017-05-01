//
// Created by root on 17-4-13.
//

#include "User.hpp"

int AMSD::User::Auth(string token, User *userinfo) {
	if (token == LocalAdminToken)
		return 0;

	Lock.lock_shared();
	auto meow = TokenCache.find(token);
	bool FoundInCache = (meow != TokenCache.end());
	Lock.unlock_shared();

	if (FoundInCache)
		return 0;

	SQLAutomator::SQLite3 *userdb = db_user.OpenSQLite3();


	userdb->Prepare("SELECT * FROM user WHERE Token = ?1");

	userdb->Bind(1, token);

	int query_rc = userdb->Step();

	delete userdb;

	if (query_rc == SQLITE_ROW) {
		Lock.lock();
		TokenCache.insert(token);
		Lock.unlock();
		if (userinfo) {

		}
		return 0;
	} else {
		return 1;
	}
}
