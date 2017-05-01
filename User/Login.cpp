//
// Created by root on 17-4-13.
//

#include "User.hpp"

int AMSD::User::Login(string user, string passwd, string &token) {
	int ret = 0;

	SQLAutomator::SQLite3 *userdb = db_user.OpenSQLite3();

	userdb->Prepare("UPDATE user SET Token = ?1 WHERE UserName = ?2 AND Password = ?3");

	token = amsd_random_string();

	u_char hashed_passwd[64];

	SHA512((const u_char *)passwd.c_str(), passwd.length(), hashed_passwd);

	userdb->Bind(1, token);
	userdb->Bind(2, user);
	userdb->Bind(3, {hashed_passwd, 64});

	userdb->Step();

	if (!sqlite3_changes(userdb->SQLite3DB))
		ret = 2;
	else {
		Lock.lock();
		TokenCache.insert(token);
		Lock.unlock();
	}

	delete userdb;


	return ret;
}
