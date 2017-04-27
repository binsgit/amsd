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

struct stCtx {
    Reimu::IPEndPoint *remoteEP;
    time_t LastDataCollection;
    json_t *j_summary;
    json_t *j_pools, *j_devices, *j_modules;

    map<int, vector<int>> modcounter;
};

static void *getSummary(void *userp){
	try {
		stCtx *thisCtx = (stCtx *) userp;

		thisCtx->j_summary = json_object();

		SQLAutomator::SQLite3 thisdb = *db_summary.OpenSQLite3();
		thisdb.Prepare("SELECT Elapsed, MHSav, Accepted, Rejected, NetworkBlocks, BestShare FROM summary "
				       "WHERE Time = ?1 AND Addr = ?2 AND Port = ?3");
		thisdb.Bind(1, thisCtx->LastDataCollection);
		thisdb.Bind(2, {thisCtx->remoteEP->Addr, thisCtx->remoteEP->AddressFamily == AF_INET ? 4 : 16});
		thisdb.Bind(3, thisCtx->remoteEP->Port);

		thisdb.Step();

		json_object_set_new(thisCtx->j_summary, "Elapsed", json_integer(thisdb.Column(0)));
		json_object_set_new(thisCtx->j_summary, "MHSav", json_real(thisdb.Column(1)));
		json_object_set_new(thisCtx->j_summary, "Accepted", json_integer(thisdb.Column(2)));
		json_object_set_new(thisCtx->j_summary, "Rejected", json_integer(thisdb.Column(3)));
		json_object_set_new(thisCtx->j_summary, "NetworkBlocks", json_integer(thisdb.Column(4)));
		json_object_set_new(thisCtx->j_summary, "BestShare", json_integer(thisdb.Column(5)));

	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

static void *getPool(void *userp){
	try {
		stCtx *thisCtx = (stCtx *) userp;

		thisCtx->j_pools = json_array();
		json_t *j_pool;

		SQLAutomator::SQLite3 thisdb = *db_pool.OpenSQLite3();
		thisdb.Prepare("SELECT PoolID, URL, StratumActive, User, Status, GetWorks, Accepted, Rejected, Stale, "
				       "LastShareTime, LastShareDifficulty FROM pool WHERE Time = ?1 "
				       "AND Addr = ?2 AND Port = ?3");
		thisdb.Bind(1, thisCtx->LastDataCollection);
		thisdb.Bind(2, {thisCtx->remoteEP->Addr, thisCtx->remoteEP->AddressFamily == AF_INET ? 4 : 16});
		thisdb.Bind(3, thisCtx->remoteEP->Port);

		while (thisdb.Step() == SQLITE_ROW) {
			j_pool = json_object();

			json_object_set_new(j_pool, "PoolID", json_integer(thisdb.Column(0)));
			json_object_set_new(j_pool, "URL", json_string(thisdb.Column(1)));
			json_object_set_new(j_pool, "StratumActive", json_integer(thisdb.Column(2)));
			json_object_set_new(j_pool, "User", json_string(thisdb.Column(3)));
			json_object_set_new(j_pool, "Status", json_string(thisdb.Column(4)));
			json_object_set_new(j_pool, "GetWorks", json_integer(thisdb.Column(5)));
			json_object_set_new(j_pool, "Accepted", json_integer(thisdb.Column(6)));
			json_object_set_new(j_pool, "Rejected", json_integer(thisdb.Column(7)));
			json_object_set_new(j_pool, "Stale", json_integer(thisdb.Column(8)));
			json_object_set_new(j_pool, "LastShareTime", json_integer(thisdb.Column(9)));
			json_object_set_new(j_pool, "LastShareDifficulty", json_integer(thisdb.Column(10)));

			json_array_append_new(thisCtx->j_pools, j_pool);
		}

	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

static void *getDevice(void *userp){
	try {
		stCtx *thisCtx = (stCtx *) userp;
		json_t *j_device;
		SQLAutomator::SQLite3 thisdb = *db_device.OpenSQLite3();
		thisdb.Prepare("SELECT ASC, Name, ID, Enabled, Status, Temperature, MHSav, MHS5s, MHS1m, MHS5m, MHS15m, "
				       "LastValidWork FROM device WHERE Time = ?1 "
				       "AND Addr = ?2 AND Port = ?3");
		thisdb.Bind(1, thisCtx->LastDataCollection);
		thisdb.Bind(2, {thisCtx->remoteEP->Addr, thisCtx->remoteEP->AddressFamily == AF_INET ? 4 : 16});
		thisdb.Bind(3, thisCtx->remoteEP->Port);

		while (thisdb.Step() == SQLITE_ROW) {
			j_device = json_object();

			int devid = thisdb.Column(2);

			json_object_set_new(j_device, "ASC", json_integer(thisdb.Column(0)));
			json_object_set_new(j_device, "Name", json_string(thisdb.Column(1)));
			json_object_set_new(j_device, "ID", json_integer(devid));
			json_object_set_new(j_device, "MMCount", json_integer((long)thisCtx->modcounter[devid].size()));
			json_object_set_new(j_device, "Enabled", json_string(thisdb.Column(3)));
			json_object_set_new(j_device, "Status", json_string(thisdb.Column(4)));
			json_object_set_new(j_device, "Temperature", json_real(thisdb.Column(5)));
			json_object_set_new(j_device, "MHSav", json_real(thisdb.Column(6)));
			json_object_set_new(j_device, "MHS5s", json_real(thisdb.Column(7)));
			json_object_set_new(j_device, "MHS1m", json_real(thisdb.Column(8)));
			json_object_set_new(j_device, "MHS5m", json_real(thisdb.Column(9)));
			json_object_set_new(j_device, "MHS15m", json_real(thisdb.Column(10)));
			json_object_set_new(j_device, "LastValidWork", json_integer(thisdb.Column(11)));

			json_array_append_new(thisCtx->j_devices, j_device);

		}
	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

static void *getModule(void *userp){
	try {
		stCtx *thisCtx = (stCtx *) userp;
		json_t *j_module, *j_echu;
		int devid, modid;
		SQLAutomator::SQLite3 thisdb = *db_module_avalon7.OpenSQLite3();
		thisdb.Prepare("SELECT DeviceID, ModuleID, LED, Elapsed, Ver, DNA, LW, DH, GHSmm, WU, Temp, TMax, Fan, "
				       "FanR, PG, ECHU_0, ECHU_1, ECHU_2, ECHU_3, ECMM FROM module_avalon7 WHERE "
				       "Time = ?1 AND Addr = ?2 AND Port = ?3");
		thisdb.Bind(1, thisCtx->LastDataCollection);
		thisdb.Bind(2, {thisCtx->remoteEP->Addr, thisCtx->remoteEP->AddressFamily == AF_INET ? 4 : 16});
		thisdb.Bind(3, thisCtx->remoteEP->Port);

		while (thisdb.Step() == SQLITE_ROW) {
			j_module = json_object();
			j_echu = json_array();
			devid = thisdb.Column(0);
			modid = thisdb.Column(1);

			thisCtx->modcounter[devid].push_back(modid);

			json_object_set_new(j_module, "DeviceID", json_integer(thisdb.Column(0)));
			json_object_set_new(j_module, "ModuleID", json_integer(thisdb.Column(1)));
			json_object_set_new(j_module, "LED", json_integer(thisdb.Column(2)));
			json_object_set_new(j_module, "Elapsed", json_integer(thisdb.Column(3)));
			json_object_set_new(j_module, "Ver", json_string(thisdb.Column(4)));

			Reimu::UniversalType *dna = thisdb.Column(5);

			char sbuf[32] = {0};
			if (dna->Size() == 8)
				snprintf(sbuf, 31, "%016" PRIx64, dna->operator uint64_t());

			json_object_set_new(j_module, "DNA", json_string(sbuf));
			json_object_set_new(j_module, "LW", json_integer(thisdb.Column(6)));
			json_object_set_new(j_module, "DH", json_real(thisdb.Column(7)));
			json_object_set_new(j_module, "GHSmm", json_real(thisdb.Column(8)));
			json_object_set_new(j_module, "WU", json_real(thisdb.Column(9)));
			json_object_set_new(j_module, "Temp", json_integer(thisdb.Column(10)));
			json_object_set_new(j_module, "TMax", json_integer(thisdb.Column(11)));
			json_object_set_new(j_module, "Fan", json_integer(thisdb.Column(12)));
			json_object_set_new(j_module, "FanR", json_integer(thisdb.Column(13)));
			json_object_set_new(j_module, "PG", json_integer(thisdb.Column(14)));

			json_array_append_new(j_echu, json_integer(thisdb.Column(15)));
			json_array_append_new(j_echu, json_integer(thisdb.Column(16)));
			json_array_append_new(j_echu, json_integer(thisdb.Column(17)));
			json_array_append_new(j_echu, json_integer(thisdb.Column(18)));

			json_object_set_new(j_module, "ECHU", j_echu);
			json_object_set_new(j_module, "ECMM", json_integer(thisdb.Column(19)));

			json_array_append_new(thisCtx->j_modules, j_module);
		}

	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

int AMSD::Operations::status(json_t *in_data, json_t *&out_data){
	json_t *j_status = json_object();
	json_t *j_ip, *j_port;

	stCtx thisCtx;

	j_ip = json_object_get(in_data, "ip");
	j_port = json_object_get(in_data, "port");

	if (!json_is_string(j_ip) || !json_is_integer(j_port))
		return -1;

	thisCtx.remoteEP = new Reimu::IPEndPoint(json_string_value(j_ip), (uint16_t)json_integer_value(j_port));
	thisCtx.LastDataCollection = RuntimeData::TimeStamp::LastDataCollection();


	pthread_t pdw[4];

	pthread_create(&pdw[0], &_pthread_detached, &getSummary, &thisCtx);
	pthread_create(&pdw[1], &_pthread_detached, &getPool, &thisCtx);
	pthread_create(&pdw[2], &_pthread_detached, &getModule, &thisCtx);
	pthread_join(pdw[2], NULL);
	pthread_create(&pdw[3], &_pthread_detached, &getDevice, &thisCtx);
	pthread_join(pdw[0], NULL);
	pthread_join(pdw[1], NULL);
	pthread_join(pdw[3], NULL);

	json_object_set_new(j_status, "Summary", thisCtx.j_summary);
	json_object_set_new(j_status, "Pools", thisCtx.j_pools);
	json_object_set_new(j_status, "Devices", thisCtx.j_devices);
	json_object_set_new(j_status, "Modules", thisCtx.j_modules);

	json_object_set_new(out_data, "Status", j_status);

	return 0;
}