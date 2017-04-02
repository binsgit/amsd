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

int AMSD::Operations::rawapi(json_t *in_data, json_t *&out_data){
	json_t *j_ip, *j_port, *j_req;


	j_ip = json_object_get(in_data, "ip");
	j_port = json_object_get(in_data, "port");
	j_req = json_object_get(in_data, "req");

	if (!json_is_string(j_ip) || !json_is_integer(j_port) || !json_is_string(j_req))
		return -1;

	string reqret;

	Reimu::IPEndPoint ep(string(json_string_value(j_ip)), (uint16_t)json_integer_value(j_port));

	int ret = CgMinerAPI::RequestRaw(ep, string(json_string_value(j_req)), reqret);

	json_object_set_new(out_data, "ret", json_string(reqret.c_str()));

	return ret;
}