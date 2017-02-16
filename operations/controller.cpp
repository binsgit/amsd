//
// Created by root on 17-2-14.
//

#include "../amsd.hpp"

using namespace mysqlpp;

int amsd_operation_controller(json_t *in_data, json_t *&out_data){
	json_t *j_op = json_object_get(in_data, "op");
	string op;
	size_t index = 0;
	json_t *j_controllers, *j_controller;
	json_t *j_con_ip, *j_con_port, *j_con_mods;
	json_t *j_errmsg;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);

	try {
		Connection mysqlconn(Config["Database"]["Database"].c_str(), Config["Database"]["Host"].c_str(),
				     Config["Database"]["User"].c_str(), Config["Database"]["Password"].c_str());


		if (op == "list") {
			Query query = mysqlconn.query("SELECT * FROM controller_config");
			StoreQueryResult res = query.store();

			j_controllers = json_array();

			for (auto it = res.begin(); it != res.end(); ++it) {
				Row row = *it;
				j_controller = json_object();
				json_object_set_new(j_controller, "ip", json_string(row[0].c_str()));
				json_object_set_new(j_controller, "port", json_integer(strtol(row[1].c_str(), NULL, 10)));
				json_object_set_new(j_controller, "mods", json_integer(strtol(row[2].c_str(), NULL, 10)));
				json_array_append_new(j_controllers, j_controller);
			}

			json_object_set_new(out_data, "controllers", j_controllers);

		} else if (op == "add") {
			j_controllers = json_object_get(in_data, "controllers");
			if (!j_controllers || !json_is_array(j_controllers))
				return -1;

			json_array_foreach(j_controllers, index, j_controller) {
				if (json_is_object(j_controller)) {
					j_con_ip = json_object_get(j_controller, "ip");
					j_con_port = json_object_get(j_controller, "port");
					j_con_mods = json_object_get(j_controller, "mods");

					if (j_con_ip && j_con_port && j_con_mods)
						if (json_is_string(j_con_ip) && json_is_integer(j_con_port) &&
						    json_is_integer(j_con_mods)) {
							Query query = mysqlconn.query();
							query << "INSERT INTO controller_config VALUES ('"
							      << json_string_value(j_con_ip)
							      << "'," << json_integer_value(j_con_port)
							      << "," << json_integer_value(j_con_mods)
							      << ")";
							query.execute();
						}
				}
			}

		} else if (op == "del") {
			j_controllers = json_object_get(in_data, "controllers");
			if (!j_controllers || !json_is_array(j_controllers))
				return -1;

			json_array_foreach(j_controllers, index, j_controller) {
				if (json_is_object(j_controller)) {
					j_con_ip = json_object_get(j_controller, "ip");
					j_con_port = json_object_get(j_controller, "port");
					j_con_mods = json_object_get(j_controller, "mods");

					if (j_con_ip && j_con_port && j_con_mods)
						if (json_is_string(j_con_ip) && json_is_integer(j_con_port) &&
						    json_is_integer(j_con_mods)) {
							Query query = mysqlconn.query();
							query << "DELETE FROM controller_config WHERE ip = "
							      << json_string_value(j_con_ip)
							      << " AND port = " << json_integer_value(j_con_port)
							      << " AND mods = " << json_integer_value(j_con_mods);
							query.execute();
						}
				}
			}

		} else if (op == "wipe") {
			Query query = mysqlconn.query("TRUNCATE controller_config");
			query.execute();

		}

		mysqlconn.disconnect();
		mysqlconn.thread_end();


	} catch (const BadQuery& e) {
		j_errmsg = json_string(e.what());
		json_object_set_new(out_data, "error", j_errmsg);
		return -20;
	} catch (const Exception& er) {
		j_errmsg = json_string(er.what());
		json_object_set_new(out_data, "error", j_errmsg);
		return -21;
	}

	return 0;
}