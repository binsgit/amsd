//
// Created by root on 17-4-21.
//

#include "User.hpp"

set<string> AMSD::User::TokenCache;
shared_timed_mutex AMSD::User::Lock;
string AMSD::User::LocalAdminToken = amsd_random_string();