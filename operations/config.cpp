//
// Created by root on 17-3-2.
//

#include "../amsd.hpp"

int amsd_operation_config(json_t *in_data, json_t *&out_data){
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

		j_value = json_string(Config[key][subkey].c_str());

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

		Config[key][subkey] = value;

		amsd_save_config("/etc/ams/config.json", 1);

		Lock_Config.unlock();



	} else {
		return -1;
	}



	return 0;
}