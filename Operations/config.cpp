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

#include "Operations.hpp"

int AMSD::Operations::config(json_t *in_data, json_t *&out_data) {
	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_key, *j_subkey, *j_value;
	string key, subkey, value;

	if (!json_is_string(j_op))
		return -1;

	const char *op = json_string_value(j_op);

	if (0 == strcmp(op, "get")) {
		j_key = json_object_get(in_data, "key");
		j_subkey = json_object_get(in_data, "subkey");

		if (!(json_is_string(j_key) && json_is_string(j_subkey)))
			return -1;

		Lock_Config.lock_shared();

		j_value = json_string(ConfigList[key][subkey].c_str());

		json_object_set_new(out_data, "value", j_value);

		Lock_Config.unlock_shared();

	} else if (0 == strcmp(op, "set")) {
		j_key = json_object_get(in_data, "key");
		j_subkey = json_object_get(in_data, "subkey");
		j_value = json_object_get(in_data, "value");

		if (!(json_is_string(j_key) && json_is_string(j_subkey) && json_is_string(j_value)))
			return -1;

		key = string(json_string_value(j_key));
		subkey = string(json_string_value(j_subkey));
		value = string(json_string_value(j_value));

		Lock_Config.lock();

		ConfigList[key][subkey] = value;

		Config::Save();

		Lock_Config.unlock();



	} else {
		return -1;
	}



	return 0;
}
