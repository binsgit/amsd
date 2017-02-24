//
// Created by root on 17-2-14.
//

#include <netinet/in.h>
#include "../amsd.hpp"

int amsd_operation_controller(json_t *in_data, json_t *&out_data){
	json_t *j_op = json_object_get(in_data, "op");
	string op;
	size_t index = 0;
	json_t *j_controllers, *j_controller;
	json_t *j_con_ip, *j_con_port;
	json_t *j_errmsg;
	sqlite3 *thisdb;
	sqlite3_stmt *stmt;
	int rc;
	const void *addr;
	int addr_len;
	char addrsbuf[INET6_ADDRSTRLEN];
	time_t timenow;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);


	db_open(dbpath_controller, thisdb);

	if (!thisdb)
		return -2;

	if (op == "list") {

		j_controllers = json_array();

	sqlite3_prepare_v2(thisdb, "SELECT * FROM controller", -1, &stmt, NULL);

	while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		j_controller = json_object();

		addr = sqlite3_column_blob(stmt, 1);
		addr_len = sqlite3_column_bytes(stmt, 1);

		if (addr_len == 4) {
			inet_ntop(AF_INET, addr, addrsbuf, INET_ADDRSTRLEN);
		} else {
			inet_ntop(AF_INET6, addr, addrsbuf, INET6_ADDRSTRLEN);
		}

		json_object_set_new(j_controller, "mtime", json_integer(sqlite3_column_int(stmt, 0)));
		json_object_set_new(j_controller, "ip", json_string(addrsbuf));
		json_object_set_new(j_controller, "port", json_integer(sqlite3_column_int(stmt, 2)));
		json_array_append_new(j_controllers, j_controller);
	}


		json_object_set_new(out_data, "controllers", j_controllers);

	} else if (op == "add") {
		j_controllers = json_object_get(in_data, "controllers");
		if (!j_controllers || !json_is_array(j_controllers))
			return -1;

		timenow = time(NULL);

		json_array_foreach(j_controllers, index, j_controller) {
			if (json_is_object(j_controller)) {
				j_con_ip = json_object_get(j_controller, "ip");
				j_con_port = json_object_get(j_controller, "port");

				if (j_con_ip && j_con_port)
					if (json_is_string(j_con_ip) && json_is_integer(j_con_port)) {
						sqlite3_prepare_v2(thisdb, "INSERT INTO controller VALUES ("
							"?1, ?2, ?3)", -1, &stmt, NULL);

						sqlite3_bind_int64(stmt, 1, timenow);
						sqlite3_bind_int64(stmt, 3, json_integer_value(j_con_port));

						const char *ipsbuf = json_string_value(j_con_ip);

						if (strchr(ipsbuf, ':')) {
							in6_addr thisaddr;
							inet_pton(AF_INET6, ipsbuf, &thisaddr);
							sqlite3_bind_blob(stmt, 2, thisaddr.__in6_u.__u6_addr8, 16, SQLITE_STATIC);
						} else {
							in_addr thisaddr;
							inet_pton(AF_INET, ipsbuf, &thisaddr);
							sqlite3_bind_blob(stmt, 2, &thisaddr.s_addr, 4, SQLITE_STATIC);
						}

						sqlite3_step(stmt);
						sqlite3_finalize(stmt);
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

				if (j_con_ip && j_con_port)
					if (json_is_string(j_con_ip) && json_is_integer(j_con_port)) {

						sqlite3_prepare_v2(thisdb, "DELETE FROM controller WHERE "
							"Addr = ?1 AND Port = ?2", -1, &stmt, NULL);

						const char *ipsbuf = json_string_value(j_con_ip);

						if (strchr(ipsbuf, ':')) {
							in6_addr thisaddr;
							inet_pton(AF_INET6, ipsbuf, &thisaddr);
							sqlite3_bind_blob(stmt, 1, thisaddr.__in6_u.__u6_addr8, 16, SQLITE_STATIC);
						} else {
							in_addr thisaddr;
							inet_pton(AF_INET, ipsbuf, &thisaddr);
							sqlite3_bind_blob(stmt, 1, &thisaddr.s_addr, 4, SQLITE_STATIC);
						}

						sqlite3_bind_int64(stmt, 2, json_integer_value(j_con_port));

						sqlite3_step(stmt);
						sqlite3_finalize(stmt);
					}
			}
		}

	} else if (op == "wipe") {
		sqlite3_exec(thisdb, "TRUNCATE controller", NULL, NULL, NULL);
	}

	db_close(thisdb);

	return 0;
}