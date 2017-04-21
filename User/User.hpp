//
// Created by root on 17-4-13.
//

#ifndef AMSD_USER_HPP
#define AMSD_USER_HPP

#include "../amsd.hpp"

namespace AMSD {
    class User {
    public:

	static set<string> TokenCache;
	static shared_timed_mutex Lock;
	static string LocalAdminToken;

	struct UserInfo {
	    string UserName;

	};

	static int Login(string user, string passwd, string &token);
	static int Auth(string token, User *userinfo);

    };


}

#endif //AMSD_USER_HPP
