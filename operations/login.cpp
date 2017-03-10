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

#include "../amsd.hpp"


int amsd_operation_login(json_t *in_data, json_t *&out_data){
	NoLoginReq_Flag;

	json_t *j_user = json_object_get(in_data, "username");
	json_t *j_passwd = json_object_get(in_data, "passwd");

	if (!json_is_string(j_user) || !json_is_string(j_passwd))
		return -1;

	string token;
	string user = string(json_string_value(j_user));
	string passwd = string(json_string_value(j_passwd));

	int login_status = amsd_user_login(user, passwd, token);

	if (login_status)
		return -2;

	json_object_set_new(out_data, "token", json_string(token.c_str()));

	return 0;
}