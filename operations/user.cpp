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


int amsd_operation_user(json_t *in_data, json_t *&out_data){

	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_user, *j_passwd;
	json_t *j_token;

	int login_status;

	string user, passwd, token;

	if (!json_is_string(j_op))
		return -1;

	string op(json_string_value(j_op));


	if (op == "add") {


	} else if (op == "upd") {

	} else if (op == "login") {

	}



}