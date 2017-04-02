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

class mController {
public:
    bool Dead = false;
    string IP;
    int Port = 0;
    double GHS = 0;
    int Mods = 0;
    double TempSum = 0, TMaxSum = 0;
};

int AMSD::Operations::farmap(json_t *in_data, json_t *&out_data){
	NoLoginReq_Flag;

	sqlite3 *db[3];
	sqlite3_stmt *stmt[3];

	json_t *j_ctls = json_array();
	json_t *j_ctl;

	void *addr;
	size_t addrlen;
	uint16_t port;
	string saddr;

	map<string, mController> ctls;
	mController *ctl;

	db_open(db_module_avalon7.DatabaseURI.c_str(), db[0]);
	sqlite3_prepare(db[0], "SELECT Addr, Port, GHSmm, Temp, TMax FROM module_avalon7 WHERE "
		"Time = ?1", -1, &stmt[0], NULL);

	sqlite3_bind_int64(stmt[0], 1, RuntimeData::TimeStamp::LastDataCollection());

	while (sqlite3_step(stmt[0]) == SQLITE_ROW) {
		addr = (void *)sqlite3_column_blob(stmt[0], 0);
		addrlen = (size_t)sqlite3_column_bytes(stmt[0], 0);
		port = (uint16_t)sqlite3_column_int(stmt[0], 1);

		saddr = strbinaddr(addr, addrlen);

		ctl = &ctls[saddr+":"+to_string(port)];

		ctl->IP = saddr;
		ctl->Port = port;
		ctl->GHS += sqlite3_column_double(stmt[0], 2);
		ctl->TempSum += sqlite3_column_int64(stmt[0], 3);
		ctl->TMaxSum += sqlite3_column_int64(stmt[0], 4);
		ctl->Mods++;
	}

	db_open(db_issue.DatabaseURI.c_str(), db[1]);
	sqlite3_prepare(db[1], "SELECT Addr, Port FROM issue WHERE Time = ?1 AND "
		"Type >= 0x10 AND Type < 0x20", -1, &stmt[1], NULL);

	sqlite3_bind_int64(stmt[1], 1, RuntimeData::TimeStamp::LastDataCollection());

	while (sqlite3_step(stmt[1]) == SQLITE_ROW) {
		addr = (void *)sqlite3_column_blob(stmt[0], 0);
		addrlen = (size_t)sqlite3_column_bytes(stmt[0], 0);
		port = (uint16_t)sqlite3_column_int(stmt[0], 1);

		saddr = strbinaddr(addr, addrlen);

		ctl = &ctls[saddr+":"+to_string(port)];

		ctl->Dead = 1;
	}

	for (auto const &it: ctls) {
		j_ctl = json_object();

		json_object_set_new(j_ctl, "IP", json_string(it.second.IP.c_str()));
		json_object_set_new(j_ctl, "Port", json_integer(it.second.Port));
		json_object_set_new(j_ctl, "GHS", json_real(it.second.GHS));
		json_object_set_new(j_ctl, "Mods", json_integer(it.second.Mods));
		json_object_set_new(j_ctl, "Temp", json_real(it.second.TempSum/it.second.Mods));
		json_object_set_new(j_ctl, "TMax", json_real(it.second.TMaxSum/it.second.Mods));
		json_object_set_new(j_ctl, "Dead", json_boolean(it.second.Dead));

		json_array_append_new(j_ctls, j_ctl);
	}


	for (int i=0; i<2; i++) {
		sqlite3_finalize(stmt[i]);
		db_close(db[i]);
	}


	json_object_set_new(out_data, "Controllers", j_ctls);

	return 0;
}