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


int AMSD::Operations::status(json_t *in_data, json_t *&out_data){
	json_t *j_status = json_object();
	json_t *j_ip, *j_port;
	json_t *j_summary, *j_pool, *j_device, *j_module;
	json_t *j_pools, *j_devices, *j_modules;
	json_t *j_echu;
	sqlite3 *db[4];
	sqlite3_stmt *stmt[4];
	const char *ip;
	int port;
	int addrlen = 4;
	int devid, modid;
	uint64_t *dna;
	char sbuf[32];
	map<int, vector<int>> modcounter;


	j_ip = json_object_get(in_data, "ip");
	j_port = json_object_get(in_data, "port");

	uint8_t addr[16];

	if (!json_is_string(j_ip) || !json_is_integer(j_port))
		return -1;

	ip = json_string_value(j_ip);
	port = (int)json_integer_value(j_port);

	if (strchr(ip, ':')) { // IPv6
		inet_pton(AF_INET6, ip, addr);
		addrlen = 8;
	} else {
		inet_pton(AF_INET, ip, addr);
	}

	// Summary

	j_summary = json_object();

//	Lock_DataCollector.lock();

	db_open(db_summary.DatabaseURI.c_str(), db[0]);

	sqlite3_prepare(db[0], "SELECT Elapsed, MHSav, Accepted, Rejected, NetworkBlocks, BestShare FROM summary "
		"WHERE Time = ?1 AND Addr = ?2 AND Port = ?3", -1, &stmt[0], NULL);

	sqlite3_bind_int64(stmt[0], 1, RuntimeData::TimeStamp::LastDataCollection());
	sqlite3_bind_blob(stmt[0], 2, addr, addrlen, SQLITE_STATIC);
	sqlite3_bind_int(stmt[0], 3, port);

	sqlite3_step(stmt[0]);

	json_object_set_new(j_summary, "Elapsed", json_integer(sqlite3_column_int64(stmt[0], 0)));
	json_object_set_new(j_summary, "MHSav", json_real(sqlite3_column_double(stmt[0], 1)));
	json_object_set_new(j_summary, "Accepted", json_integer(sqlite3_column_int64(stmt[0], 2)));
	json_object_set_new(j_summary, "Rejected", json_integer(sqlite3_column_int64(stmt[0], 3)));
	json_object_set_new(j_summary, "NetworkBlocks", json_integer(sqlite3_column_int64(stmt[0], 4)));
	json_object_set_new(j_summary, "BestShare", json_integer(sqlite3_column_int64(stmt[0], 5)));

	// Pool

	j_pools = json_array();

	db_open(db_pool.DatabaseURI.c_str(), db[1]);

	sqlite3_prepare(db[1], "SELECT PoolID, URL, StratumActive, User, Status, GetWorks, Accepted, Rejected, Stale, "
		"LastShareTime, LastShareDifficulty FROM pool WHERE Time = ?1 "
		"AND Addr = ?2 AND Port = ?3", -1, &stmt[1], NULL);

	sqlite3_bind_int64(stmt[1], 1, RuntimeData::TimeStamp::LastDataCollection());
	sqlite3_bind_blob(stmt[1], 2, addr, addrlen, SQLITE_STATIC);
	sqlite3_bind_int(stmt[1], 3, port);

	while (sqlite3_step(stmt[1]) == SQLITE_ROW) {

		j_pool = json_object();

		json_object_set_new(j_pool, "PoolID", json_integer(sqlite3_column_int64(stmt[1], 0)));
		json_object_set_new(j_pool, "URL", json_string((const char *) sqlite3_column_text(stmt[1], 1)));
		json_object_set_new(j_pool, "StratumActive", json_integer(sqlite3_column_int64(stmt[1], 2)));
		json_object_set_new(j_pool, "User", json_string((const char *) sqlite3_column_text(stmt[1], 3)));
		json_object_set_new(j_pool, "Status", json_string((const char *) sqlite3_column_text(stmt[1], 4)));
		json_object_set_new(j_pool, "GetWorks", json_integer(sqlite3_column_int64(stmt[1], 5)));
		json_object_set_new(j_pool, "Accepted", json_integer(sqlite3_column_int64(stmt[1], 6)));
		json_object_set_new(j_pool, "Rejected", json_integer(sqlite3_column_int64(stmt[1], 7)));
		json_object_set_new(j_pool, "Stale", json_integer(sqlite3_column_int64(stmt[1], 8)));
		json_object_set_new(j_pool, "LastShareTime", json_integer(sqlite3_column_int64(stmt[1], 9)));
		json_object_set_new(j_pool, "LastShareDifficulty", json_integer(sqlite3_column_int64(stmt[1], 10)));

		json_array_append_new(j_pools, j_pool);
	}

	// Device & Module

	j_devices = json_array();
	j_modules = json_array();

	db_open(db_device.DatabaseURI.c_str(), db[2]);

	sqlite3_prepare(db[2], "SELECT ASC, Name, ID, Enabled, Status, Temperature, MHSav, MHS5s, MHS1m, MHS5m, MHS15m, "
		"LastValidWork FROM device WHERE Time = ?1 "
		"AND Addr = ?2 AND Port = ?3", -1, &stmt[2], NULL);

	sqlite3_bind_int64(stmt[2], 1, RuntimeData::TimeStamp::LastDataCollection());
	sqlite3_bind_blob(stmt[2], 2, addr, addrlen, SQLITE_STATIC);
	sqlite3_bind_int(stmt[2], 3, port);

	db_open(db_module_avalon7.DatabaseURI.c_str(), db[3]);

	sqlite3_prepare(db[3], "SELECT DeviceID, ModuleID, LED, Elapsed, Ver, DNA, LW, DH, GHSmm, WU, Temp, TMax, Fan, "
		"FanR, PG, ECHU_0, ECHU_1, ECHU_2, ECHU_3, ECMM FROM module_avalon7 WHERE "
		"Time = ?1 AND Addr = ?2 AND Port = ?3", -1, &stmt[3], NULL);

	sqlite3_bind_int64(stmt[3], 1, RuntimeData::TimeStamp::LastDataCollection());
	sqlite3_bind_blob(stmt[3], 2, addr, addrlen, SQLITE_STATIC);
	sqlite3_bind_int(stmt[3], 3, port);


	while (sqlite3_step(stmt[3]) == SQLITE_ROW) {
		j_module = json_object();
		j_echu = json_array();
		devid = sqlite3_column_int(stmt[3], 0);
		modid = sqlite3_column_int(stmt[3], 1);

		modcounter[devid].push_back(modid);

		json_object_set_new(j_module, "DeviceID", json_integer(sqlite3_column_int64(stmt[3], 0)));
		json_object_set_new(j_module, "ModuleID", json_integer(sqlite3_column_int64(stmt[3], 1)));
		json_object_set_new(j_module, "LED", json_integer(sqlite3_column_int64(stmt[3], 2)));
		json_object_set_new(j_module, "Elapsed", json_integer(sqlite3_column_int64(stmt[3], 3)));
		json_object_set_new(j_module, "Ver", json_string((const char *)sqlite3_column_text(stmt[3], 4)));

		dna = (uint64_t *)sqlite3_column_blob(stmt[3], 5);
		snprintf(sbuf, 31, "%016" PRIx64, *dna);
		json_object_set_new(j_module, "DNA", json_string(sbuf));
		json_object_set_new(j_module, "LW", json_integer(sqlite3_column_int64(stmt[3], 6)));
		json_object_set_new(j_module, "DH", json_real(sqlite3_column_double(stmt[3], 7)));
		json_object_set_new(j_module, "GHSmm", json_real(sqlite3_column_double(stmt[3], 8)));
		json_object_set_new(j_module, "WU", json_real(sqlite3_column_double(stmt[3], 9)));
		json_object_set_new(j_module, "Temp", json_integer(sqlite3_column_int64(stmt[3], 10)));
		json_object_set_new(j_module, "TMax", json_integer(sqlite3_column_int64(stmt[3], 11)));
		json_object_set_new(j_module, "Fan", json_integer(sqlite3_column_int64(stmt[3], 12)));
		json_object_set_new(j_module, "FanR", json_integer(sqlite3_column_int64(stmt[3], 13)));
		json_object_set_new(j_module, "PG", json_integer(sqlite3_column_int64(stmt[3], 14)));

		json_array_append_new(j_echu, json_integer(sqlite3_column_int64(stmt[3], 15)));
		json_array_append_new(j_echu, json_integer(sqlite3_column_int64(stmt[3], 16)));
		json_array_append_new(j_echu, json_integer(sqlite3_column_int64(stmt[3], 17)));
		json_array_append_new(j_echu, json_integer(sqlite3_column_int64(stmt[3], 18)));

		json_object_set_new(j_module, "ECHU", j_echu);
		json_object_set_new(j_module, "ECMM", json_integer(sqlite3_column_int64(stmt[3], 19)));

		json_array_append_new(j_modules, j_module);
	}

	while (sqlite3_step(stmt[2]) == SQLITE_ROW) {
		j_device = json_object();

		devid = sqlite3_column_int(stmt[2], 2);

		json_object_set_new(j_device, "ASC", json_integer(sqlite3_column_int64(stmt[2], 0)));
		json_object_set_new(j_device, "Name", json_string((const char *) sqlite3_column_text(stmt[2], 1)));
		json_object_set_new(j_device, "ID", json_integer(devid));
		json_object_set_new(j_device, "MMCount", json_integer((long)modcounter[devid].size()));
		json_object_set_new(j_device, "Enabled", json_string((const char *) sqlite3_column_text(stmt[2], 3)));
		json_object_set_new(j_device, "Status", json_string((const char *) sqlite3_column_text(stmt[2], 4)));
		json_object_set_new(j_device, "Temperature", json_real(sqlite3_column_double(stmt[2], 5)));
		json_object_set_new(j_device, "MHSav", json_real(sqlite3_column_double(stmt[2], 6)));
		json_object_set_new(j_device, "MHS5s", json_real(sqlite3_column_double(stmt[2], 7)));
		json_object_set_new(j_device, "MHS1m", json_real(sqlite3_column_double(stmt[2], 8)));
		json_object_set_new(j_device, "MHS5m", json_real(sqlite3_column_double(stmt[2], 9)));
		json_object_set_new(j_device, "MHS15m", json_real(sqlite3_column_double(stmt[2], 10)));
		json_object_set_new(j_device, "LastValidWork", json_integer(sqlite3_column_int64(stmt[2], 11)));

		json_array_append_new(j_devices, j_device);

	}

	json_object_set_new(j_status, "Summary", j_summary);
	json_object_set_new(j_status, "Pools", j_pools);
	json_object_set_new(j_status, "Devices", j_devices);
	json_object_set_new(j_status, "Modules", j_modules);

	json_object_set_new(out_data, "Status", j_status);

	for (int j=0; j<4; j++) {
		sqlite3_finalize(stmt[j]);
		db_close(db[j]);
	}

//	Lock_DataCollector.unlock();

	return 0;
}