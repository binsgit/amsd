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


static shared_timed_mutex GlobalLock;

class ctl_info_ctx {
public:
    string addr;
    time_t elapsed;
    string pool_url;
    string pool1_url;
    string pool2_url;
    string pool_worker;
    string pool1_worker;
    string pool2_worker;
    uint mods_count;
    string mod_type;
    double mhs;
    double mhsav;
};

static map<string, ctl_info_ctx> CtlInfoRaw;
static shared_timed_mutex CtlInfoRaw_Lock;

static multimap<time_t, uint> sortby_elapsed;
static multimap<string, uint> sortby_pool_url;
static multimap<string, uint> sortby_pool_worker;
static multimap<uint, uint> sortby_mods_count;
static multimap<string, uint> sortby_mod_type;
static multimap<double, uint> sortby_mhs;
static multimap<double, uint> sortby_mhsav;


static void *getPoolData(void *userp){
	try {
		SQLAutomator::SQLite3 *thisdb = db_pool.OpenSQLite3();
		thisdb->Prepare("SELECT Addr, Port, URL, User, PoolID from pool WHERE Time = ?1");

		thisdb->Bind(1, RuntimeData::TimeStamp::LastDataCollection());

		void *remote_inaddr;
		uint16_t remote_port;

		while (thisdb->Step() == SQLITE_ROW) {
			remote_inaddr = (void *)sqlite3_column_blob(thisdb->SQLite3Statement, 0);
			remote_port = (uint16_t)sqlite3_column_int(thisdb->SQLite3Statement, 1);

			IPEndPoint thisep(remote_inaddr, 4, remote_port);

			int poolid = thisdb->Column(4);
			string addr = thisep.ToString();

			CtlInfoRaw_Lock.lock();
			ctl_info_ctx &tgtctx = CtlInfoRaw[addr];
			tgtctx.addr = addr;

			if (poolid == 0) {
				tgtctx.pool_url = thisdb->Column(2).operator std::string();
				tgtctx.pool_worker = thisdb->Column(3).operator std::string();
			} else if (poolid == 1) {
				tgtctx.pool1_url = thisdb->Column(2).operator std::string();
				tgtctx.pool1_worker = thisdb->Column(3).operator std::string();
			} if (poolid == 2) {
				tgtctx.pool2_url = thisdb->Column(2).operator std::string();
				tgtctx.pool2_worker = thisdb->Column(3).operator std::string();
			}


			CtlInfoRaw_Lock.unlock();

		}

	} catch (Reimu::Exception e) {

	}
}

static void *getModData(void *userp){
	try {
		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();
		thisdb->Prepare("SELECT Addr, Port, Ver FROM module_avalon7 WHERE Time = ?1");

		thisdb->Bind(1, RuntimeData::TimeStamp::LastDataCollection());

		void *remote_inaddr;
		uint16_t remote_port;

		ctl_info_ctx thisctx;

		while (thisdb->Step() == SQLITE_ROW) {
			remote_inaddr = (void *)sqlite3_column_blob(thisdb->SQLite3Statement, 0);
			remote_port = (uint16_t)sqlite3_column_int(thisdb->SQLite3Statement, 1);

			IPEndPoint thisep(remote_inaddr, 4, remote_port);

			thisctx.addr = thisep.ToString();
			thisctx.mod_type = thisdb->Column(2).operator std::string();


			CtlInfoRaw_Lock.lock();
			ctl_info_ctx &tgtctx = CtlInfoRaw[thisctx.addr];
			if (tgtctx.mod_type.empty() && thisctx.mod_type.size() >= 3)
				tgtctx.mod_type = thisctx.mod_type.substr(0,3);
			tgtctx.mods_count++;
			CtlInfoRaw_Lock.unlock();

		}

	} catch (Reimu::Exception e) {

	}
}


static void *getSummary(void *userp){
	try {
		SQLAutomator::SQLite3 *thisdb = db_summary.OpenSQLite3();
		thisdb->Prepare("SELECT Addr, Port, Elapsed, MHS5s, MHSav FROM summary WHERE Time = ?1");

		thisdb->Bind(1, RuntimeData::TimeStamp::LastDataCollection());

		void *remote_inaddr;
		uint16_t remote_port;
		ctl_info_ctx thisctx;

		while (thisdb->Step() == SQLITE_ROW) {
			remote_inaddr = (void *)sqlite3_column_blob(thisdb->SQLite3Statement, 0);
			remote_port = (uint16_t)sqlite3_column_int(thisdb->SQLite3Statement, 1);

			IPEndPoint thisep(remote_inaddr, 4, remote_port);

			thisctx.addr = thisep.ToString();
			thisctx.elapsed = thisdb->Column(2);
			thisctx.mhs = thisdb->Column(3);
			thisctx.mhsav = thisdb->Column(4);


			CtlInfoRaw_Lock.lock();
			ctl_info_ctx &tgtctx = CtlInfoRaw[thisctx.addr];
			tgtctx.elapsed = thisctx.elapsed;
			tgtctx.mhs = thisctx.mhs;
			tgtctx.mhsav = thisctx.mhsav;
			CtlInfoRaw_Lock.unlock();

		}

	} catch (Reimu::Exception e) {

	}
}

static void sortData() {

	size_t j = 0;

	for (auto &thisctx : CtlInfoRaw) {

		sortby_elapsed.insert(pair<time_t, uint>(thisctx.second.elapsed, j));
		sortby_pool_url.insert(pair<string, uint>(thisctx.second.pool_url, j));
		sortby_pool_worker.insert(pair<string, uint>(thisctx.second.pool_worker, j));
		sortby_mod_type.insert(pair<string, uint>(thisctx.second.mod_type, j));
		sortby_mods_count.insert(pair<time_t, uint>(thisctx.second.mods_count, j));
		sortby_mhs.insert(pair<double, uint>(thisctx.second.mhs, j));
		sortby_mhsav.insert(pair<double, uint>(thisctx.second.mhsav, j));

		j++;
	}

}

static void collectData() {

	sortby_elapsed.clear();
	sortby_pool_url.clear();
	sortby_pool_worker.clear();
	sortby_mod_type.clear();
	sortby_mods_count.clear();
	sortby_mhs.clear();
	sortby_mhsav.clear();
	CtlInfoRaw.clear();



	pthread_t tid_getpool, tid_getmodata, tid_getsummary;

	pthread_create(&tid_getpool, NULL, &getPoolData, NULL);
	pthread_create(&tid_getmodata, NULL, &getModData, NULL);
	pthread_create(&tid_getsummary, NULL, &getSummary, NULL);

	pthread_join(tid_getmodata, NULL);
	pthread_join(tid_getpool, NULL);
	pthread_join(tid_getsummary, NULL);

	sortData();
}

int AMSD::Operations::poool(json_t *in_data, json_t *&out_data){

	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_nodes;
	json_t *j_poolcfg;
	json_t *j_addr;
	json_t *j_pool1url, *j_pool1user, *j_pool1pw, *j_pool2url, *j_pool2user, *j_pool2pw;
	json_t *j_pool3url, *j_pool3user, *j_pool3pw;
	json_t *j_pd_tbl_entry;
	json_t *j_pd_tbl;
	json_t *j_pd;
	json_t *j_pd_sortinfos;
	json_t *j_pd_sortinfo;

	string shell_cmd;


	if (!json_is_string(j_op))
		return -1;

	string op(json_string_value(j_op));

	if (op == "info") {
		GlobalLock.lock();

		collectData();

		j_pd_tbl = json_array();
		j_pd_sortinfos = json_object();

		for (auto &thisctxx : CtlInfoRaw) {
			auto &tsctx = thisctxx.second;
			j_pd_tbl_entry = json_object();
			json_object_set_new(j_pd_tbl_entry, "target", json_string(tsctx.addr.c_str()));
			json_object_set_new(j_pd_tbl_entry, "elapsed", json_integer(tsctx.elapsed));
			json_object_set_new(j_pd_tbl_entry, "pool_url", json_string(tsctx.pool_url.c_str()));
			json_object_set_new(j_pd_tbl_entry, "pool1_url", json_string(tsctx.pool1_url.c_str()));
			json_object_set_new(j_pd_tbl_entry, "pool2_url", json_string(tsctx.pool2_url.c_str()));
			json_object_set_new(j_pd_tbl_entry, "pool_worker", json_string(tsctx.pool_worker.c_str()));
			json_object_set_new(j_pd_tbl_entry, "pool1_worker", json_string(tsctx.pool1_worker.c_str()));
			json_object_set_new(j_pd_tbl_entry, "pool2_worker", json_string(tsctx.pool2_worker.c_str()));
			json_object_set_new(j_pd_tbl_entry, "mhs", json_real(tsctx.mhs));
			json_object_set_new(j_pd_tbl_entry, "mhsav", json_real(tsctx.mhsav));
			json_object_set_new(j_pd_tbl_entry, "mod_type", json_string(tsctx.mod_type.c_str()));
			json_object_set_new(j_pd_tbl_entry, "mods_count", json_integer(tsctx.mods_count));

			json_array_append_new(j_pd_tbl, j_pd_tbl_entry);
		}

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_mhs) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "mhs", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_mhsav) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "mhsav", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_elapsed) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "elapsed", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_pool_url) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "pool_url", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_pool_worker) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "pool_worker", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_mod_type) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "mod_type", j_pd_sortinfo);

		j_pd_sortinfo = json_array();
		for (auto &thisctxx : sortby_mods_count) {
			auto tsx = thisctxx.second;
			json_array_append_new(j_pd_sortinfo, json_integer(tsx));
		}
		json_object_set_new(j_pd_sortinfos, "mods_count", j_pd_sortinfo);

		json_object_set_new(out_data, "table", j_pd_tbl);
		json_object_set_new(out_data, "sortinfo", j_pd_sortinfos);

		GlobalLock.unlock();
	} else if (op == "modify") {
		j_nodes = json_object_get(in_data, "nodes");
		j_poolcfg = json_object_get(in_data, "poolcfg");

		if (!json_is_array(j_nodes) || !json_is_object(j_poolcfg))
			return -1;

		j_pool1url = json_object_get(j_poolcfg, "pool1url");
		j_pool1user = json_object_get(j_poolcfg, "pool1user");
		j_pool1pw = json_object_get(j_poolcfg, "pool1pw");
		j_pool2url = json_object_get(j_poolcfg, "pool2url");
		j_pool2user = json_object_get(j_poolcfg, "pool2user");
		j_pool2pw = json_object_get(j_poolcfg, "pool2pw");
		j_pool3url = json_object_get(j_poolcfg, "pool3url");
		j_pool3user = json_object_get(j_poolcfg, "pool3user");
		j_pool3pw = json_object_get(j_poolcfg, "pool3pw");

		shell_cmd += "#!/bin/sh\n";

		if (json_is_string(j_pool1url)) {
			shell_cmd += "uci set cgminer.default.pool1url='";
			shell_cmd += json_string_value(j_pool1url);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool1url";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool1user)) {
			shell_cmd += "uci set cgminer.default.pool1user='";
			shell_cmd += json_string_value(j_pool1user);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool1user'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool1pw)) {
			shell_cmd += "uci set cgminer.default.pool1pw='";
			shell_cmd += json_string_value(j_pool1pw);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool1pw'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2url)) {
			shell_cmd += "uci set cgminer.default.pool2url='";
			shell_cmd += json_string_value(j_pool2url);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool2url'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2user)) {
			shell_cmd += "uci set cgminer.default.pool2user='";
			shell_cmd += json_string_value(j_pool2user);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool2user'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2pw)) {
			shell_cmd += "uci set cgminer.default.pool2pw='";
			shell_cmd += json_string_value(j_pool2pw);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool2pw'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool3url)) {
			shell_cmd += "uci set cgminer.default.pool3url='";
			shell_cmd += json_string_value(j_pool3url);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool3url'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool3user)) {
			shell_cmd += "uci set cgminer.default.pool3user='";
			shell_cmd += json_string_value(j_pool3user);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool3user'";
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool3pw)) {
			shell_cmd += "uci set cgminer.default.pool3pw='";
			shell_cmd += json_string_value(j_pool3pw);
			shell_cmd += "'\n";
		} else {
			shell_cmd += "uci delete cgminer.default.pool3pw'";
			shell_cmd += "'\n";
		}

		shell_cmd += "uci commit\n";
		shell_cmd += "/etc/init.d/cgminer restart\n";

		size_t j;

		Lock_SuperRTACTasksList.lock();
		json_array_foreach(j_nodes, j, j_addr) {
			if (json_is_string(j_addr)) {
				SuperRTACSession *srtac = new SuperRTACSession;
				string s_ipbuf = json_string_value(j_addr);
				srtac->IP = s_ipbuf;
				srtac->ScriptName = "修改矿池信息";
				srtac->Script = shell_cmd;
				srtac->UUID = amsd_random_string();
				srtac->StartTime = time(NULL);

				srtac->Exec();
				SuperRTACTasksList.insert(pair<string, SuperRTACSession *>(s_ipbuf, srtac));
			}
		}
		Lock_SuperRTACTasksList.unlock();

	}

	return 0;
}