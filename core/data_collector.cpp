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

static const string api_cmd_summary = "{\"command\":\"summary\"}";
static const string api_cmd_estats = "{\"command\":\"estats\"}";
static const string api_cmd_edevs = "{\"command\":\"edevs\"}";
static const string api_cmd_pools = "{\"command\":\"pools\"}";



shared_timed_mutex Lock_DataCollector;

size_t amsd_datacollection_interval = 120;
struct timeval amsd_datacollection_conntimeout = {15, 0};

sqlite3_stmt *stmt_log_timeout;


static void event_cb(struct bufferevent *bev, short events, void *ptr){

}

static void conn_readcb(struct bufferevent *bev, void *user_data){
//	fprintf(stderr,"amsd: datacollector: buffer event %p ready for reading\n", bev);
	CgMinerAPIProcessor *apibuf = (CgMinerAPIProcessor *)user_data;
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);
	size_t n;

	char buf[4096];

	if (input_len > 0) {
		while (1) {
			n = bufferevent_read(bev, buf, sizeof(buf));
//			fprintf(stderr,"amsd: datacollector: buffer event %p read %zu bytes\n", bev, n);
			if (n == 0)
				break;
			apibuf->NetIOBuf.insert(apibuf->NetIOBuf.end(), buf, buf + n);
		}

	}

}

static void conn_writecb(struct bufferevent *bev, void *user_data){
//	fprintf(stderr,"amsd: datacollector: buffer event %p ready for writing\n", bev);

}

static void amsd_datacollector_instance(){
	struct event_base *eventbase = event_base_new();
	struct bufferevent *bebuf = NULL;
	uint8_t remote_sockaddr[sizeof(struct sockaddr_in6)];

	sqlite3 *thisdb;
	sqlite3_stmt *stmt;
	CgMinerAPIProcessor *abuf;
	const void *remote_inaddr;
	uint16_t remote_port;
	int inaddr_len, sockaddr_len;
	char addrsbuf[INET6_ADDRSTRLEN];
	int rc;
	sqlite3 *db_handles[5] = {0};

	string stmout = Config["DataCollector"]["ConnTimeout"];

	amsd_datacollection_conntimeout.tv_usec = 0;
	amsd_datacollection_conntimeout.tv_sec = (size_t)strtol(stmout.c_str(), NULL, 10);

	if (!amsd_datacollection_conntimeout.tv_sec) {
		fprintf(stderr, "amsd: datacollector: WARNING: Bad `ConnTimeout' in config file! Please fix!\n");
		fprintf(stderr, "amsd: datacollector: WARNING: Using 30 seconds as ConnTimeout!\n");
		amsd_datacollection_conntimeout.tv_sec = 30;
	}


	db_open(dbpath_summary, db_handles[0]);
	db_open(dbpath_module_avalon7, db_handles[1]);
	db_open(dbpath_device, db_handles[2]);
	db_open(dbpath_pool, db_handles[3]);
	db_open(dbpath_issue, db_handles[4]);


	for (size_t j = 0; j < sizeof(db_handles)/sizeof(sqlite3 *); j++) {
		if (db_handles[j]) {
			sqlite3_exec(db_handles[j], "BEGIN", NULL, NULL, NULL);
		}
	}

	sqlite3_prepare_v2(db_handles[4], "INSERT INTO issue (Time, Addr, Port, Type) VALUES (?1, ?2, ?3, ?4)",
			   -1, &stmt_log_timeout, NULL);


	db_open(dbpath_controller, thisdb);

	sqlite3_prepare_v2(thisdb, "SELECT * FROM controller", -1, &stmt, NULL);

	while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW ) {

		remote_inaddr = sqlite3_column_blob(stmt, 1);
		inaddr_len = sqlite3_column_bytes(stmt, 1);
		remote_port = (uint16_t)sqlite3_column_int(stmt, 2);

		if (inaddr_len == 4) {
			((struct sockaddr_in *)remote_sockaddr)->sin_family = AF_INET;
			((struct sockaddr_in *)&remote_sockaddr)->sin_port = htons(remote_port);
			memcpy(&((struct sockaddr_in *)remote_sockaddr)->sin_addr, remote_inaddr, 4);
			sockaddr_len = sizeof(struct sockaddr_in);
		} else if (inaddr_len == 16) {
			((struct sockaddr_in6 *)remote_sockaddr)->sin6_family = AF_INET6;
			((struct sockaddr_in6 *)&remote_sockaddr)->sin6_port = htons(remote_port);
			memcpy(&((struct sockaddr_in6 *)remote_sockaddr)->sin6_addr, remote_inaddr, 16);
			sockaddr_len = sizeof(struct sockaddr_in6);
		} else {
			// TODO: Internal err handling
			continue;
		}

		for (int apicat = 1; apicat <= 4; apicat++) {
			bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);
			bufferevent_set_timeouts(bebuf, &amsd_datacollection_conntimeout, &amsd_datacollection_conntimeout);

			if (bufferevent_socket_connect(bebuf, (struct sockaddr *)remote_sockaddr, sockaddr_len) < 0) {
				bufferevent_free(bebuf);
				continue;
			} else {
				abuf = new CgMinerAPIProcessor((CgMinerAPIProcessor::CgMiner_APIType)apicat, db_handles, RuntimeData::TimeStamp::CurrentDataCollection(), remote_inaddr, (size_t)inaddr_len, remote_port);
				bufferevent_setcb(bebuf, conn_readcb, conn_writecb, event_cb, abuf);
				bufferevent_enable(bebuf, EV_READ | EV_WRITE);
			}
		}

	}

	sqlite3_finalize(stmt);

	db_close(thisdb);

	event_base_dispatch(eventbase);
	event_base_free(eventbase);

	sqlite3_finalize(stmt_log_timeout);

	for (size_t j = 0; j < sizeof(db_handles)/sizeof(sqlite3 *); j++) {
		if (db_handles[j]) {
			sqlite3_exec(db_handles[j], "COMMIT", NULL, NULL, NULL);
			db_close(db_handles[j]);
		}
	}

}

static void *amsd_datacollector_thread(void *meow){
	timeval tv_begin, tv_end, tv_diff;

	string sintvl = Config["DataCollector"]["CollectInterval"];

	while (1) {
		amsd_datacollection_interval = (size_t)strtol(sintvl.c_str(), NULL, 10);

		if (!amsd_datacollection_interval) {
			fprintf(stderr, "amsd: datacollector: WARNING: Bad `CollectInterval' in config file! Please fix!\n");
			fprintf(stderr, "amsd: datacollector: WARNING: Using 15 minutes as CollectInterval!\n");
			amsd_datacollection_interval = 60 * 15;
		}

		Lock_DataCollector.lock();
		fprintf(stderr, "amsd: datacollector: collector instance started\n");
		gettimeofday(&tv_begin, NULL);
		RuntimeData::TimeStamp::CurrentDataCollection(time(NULL));
		amsd_datacollector_instance();
		RuntimeData::TimeStamp::LastDataCollection(RuntimeData::TimeStamp::CurrentDataCollection());
		gettimeofday(&tv_end, NULL);
		timersub(&tv_end, &tv_begin, &tv_diff);
		Lock_DataCollector.unlock();
		fprintf(stderr, "amsd: datacollector: collector instance done (%zu.%zu secs), sleeping %zu secs...\n", tv_diff.tv_sec,
			tv_diff.tv_usec, amsd_datacollection_interval);
		sleep((uint)amsd_datacollection_interval);
	}
}

void amsd_datacollector(){
	pthread_t tid;
	pthread_create(&tid, &_pthread_detached, &amsd_datacollector_thread, NULL);
	fprintf(stderr, "amsd: datacollector: collector thread started\n");
}

static const CgMinerAPIQueryAutomator aq_summary("summary", {"Elapsed", "MHS av", "MHS 5s", "MHS 1m", "MHS 5m", "MHS 15m",
							     "Found Blocks", "Getworks", "Accepted", "Rejected", "Hardware Errors",
							     "Utility", "Discarded", "Stale", "Get Failures", "Local Work",
							     "Remote Failures", "Network Blocks", "Total MH", "Work Utility",
							     "Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
							     "Best Share", "Device Hardware%", "Device Rejected%", "Pool Rejected%",
							     "Pool Stale%", "Last getwork"});

static const CgMinerAPIQueryAutomator aq_pool("pool", {"POOL", "URL", "Status", "Priority", "Quota", "Long Poll", "Getworks",
						       "Accepted", "Rejected", "Works", "Discarded", "Stale", "Get Failures",
						       "Remote Failures", "User", "Last Share Time", "Diff1 Shares", "Proxy Type",
						       "Proxy", "Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
						       "Last Share Difficulty", "Work Difficulty", "Has Stratum",
						       "Stratum Active", "Stratum URL", "Stratum Difficulty", "Has GBT",
						       "Best Share", "Pool Rejected%", "Pool Stale%", "Bad Work",
						       "Current Block Height", "Current Block Version"});

static const CgMinerAPIQueryAutomator aq_device("device", {"ASC", "Name", "ID", "Enabled", "Status", "Temperature", "MHS av",
							   "MHS 5s", "MHS 1m", "MHS 5m", "MHS 15m", "Accepted", "Rejected",
							   "Hardware Errors", "Utility", "Last Share Pool", "Last Share Time",
							   "Total MH", "Diff1 Work", "Difficulty Accepted", "Difficulty Rejected",
							   "Last Share Difficulty", "No Device", "Last Valid Work",
							   "Device Hardware%", "Device Rejected%", "Device Elapsed"});



CgMinerAPIQueryAutomator::CgMinerAPIQueryAutomator(string table_name, vector<string> json_keys) {
	TableName = table_name;
	JsonKeys = json_keys;
}

vector<string> CgMinerAPIQueryAutomator::GetJsonKeys() {
	return JsonKeys;
}

string CgMinerAPIQueryAutomator::GetInsertStmt() {
	if (InsertStmt == "") {
		InsertStmt = "INSERT INTO " + TableName + " VALUES (?1";
		for (size_t i = 2; i <= JsonKeys.size()+3; i++) {
			InsertStmt += ", ?" + to_string(i);
		}
		InsertStmt += ")";
	}

	return InsertStmt;
}


CgMinerAPIProcessor::CgMinerAPIProcessor(CgMinerAPIProcessor::CgMiner_APIType t, sqlite3 **db_handles, time_t tm, const void *addr, size_t addrlen, uint16_t port) {
	APIType = t;
	StartTime = tm;
	DBHandles = db_handles;
	Remote_AddrLen = addrlen;
	memcpy(Remote_Addr, addr, addrlen);
	Remote_Port = port;

	char addrbuf[64];
	inet_ntop(Remote_AddrLen == 4 ? AF_INET : AF_INET6, addr, addrbuf, Remote_AddrLen == 4 ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
	snprintf(addrbuf+strlen(addrbuf), 6, ":%" PRIu16, port);
	Remote_AddrText = string(addrbuf);
}

void CgMinerAPIProcessor::Process() {
	fprintf(stderr, "amsd: datacollector: CgMinerAPIProcessor: processing %s for %s\n",
		APITypeString(APIType), Remote_AddrText.c_str());

	unsigned char *apidata = &NetIOBuf[0];

	json_error_t jerror;

	j_apidata_root = json_loads((const char *)apidata, 0, &jerror);

	if (!j_apidata_root) {
		// TODO
		fprintf(stderr, "amsd: CgMinerAPIProcessor: error processing %s for %s: json error at %p: line %d: %s\n",
			APITypeString(APIType), Remote_AddrText.c_str(), this, jerror.line, jerror.text);
		return;
	}

	WriteDatabase();

	json_decref(j_apidata_root);

}


void CgMinerAPIProcessor::WriteDatabase() {
	sqlite3 *thisdb;

	switch (APIType) {
		case Summary:
			ProcessData("SUMMARY", DBHandles[0], aq_summary);
			break;
		case EStats:
			ProcessHolyShittyCrap(); // The API output is NOT easy to parse nor easy to read!!!
			break;
		case EDevs:
			ProcessData("DEVS", DBHandles[2], aq_device);
			break;
		case Pools:
			ProcessData("POOLS", DBHandles[3], aq_pool);
			break;
	}

}


void CgMinerAPIProcessor::ProcessData(const char *api_obj_name, sqlite3 *db, CgMinerAPIQueryAutomator aq) {
	json_t *j_apidata_array;
	string sql_stmt = aq.GetInsertStmt();
	vector<string> jentries = aq.GetJsonKeys();
	json_t *j_apidata;
	size_t j;
	sqlite3_stmt *stmt = NULL;
	int narg;


	j_apidata_array = json_object_get(j_apidata_root, api_obj_name);

	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}


	json_array_foreach(j_apidata_array, j, j_apidata) {

		if (!json_is_object(j_apidata)) {
			// TODO
			goto end;
		}

		sqlite3_prepare_v2(db, sql_stmt.c_str(), -1, &stmt, NULL);

		if (!stmt)
			goto end;

		bi(1, StartTime);
		bb(2, Remote_Addr,Remote_AddrLen);
		bi(3, Remote_Port);

		narg = 4;

		for (auto const &key: jentries) {
			json_t *buf0 = json_object_get(j_apidata, key.c_str());

			if (!buf0) {
				// TODO
				goto end;
			}

			if (json_is_integer(buf0))
				bi(narg, json_integer_value(buf0));
			else if (json_is_string(buf0))
				bt(narg, json_string_value(buf0));
			else if (json_is_real(buf0))
				bd(narg, json_real_value(buf0));
			else if (json_is_boolean(buf0))
				bi(narg, json_boolean_value(buf0));

			narg++;
		}

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		stmt = NULL;

	}

	end:
	if (stmt)
		sqlite3_finalize(stmt);

	return;


}

void CgMinerAPIProcessor::ProcessHolyShittyCrap() { // Special function designed to get rid of that shitty API output
	// We are not using regex because it is too SLOW.
	json_t *j_apidata_array = json_object_get(j_apidata_root, "STATS");
	sqlite3 *thisdb = DBHandles[1];
	sqlite3_stmt *stmt;
	Avalon_MM mmmm;


	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}


	if (!thisdb) {
		// TODO
		return;
	}

	json_t *j_apidata;
	size_t j;

	json_array_foreach(j_apidata_array, j, j_apidata) {

		if (!json_is_object(j_apidata)) {
			// TODO
			continue;
		}

		json_t *j_mm_count = json_object_get(j_apidata, "MM Count");

		if (!json_is_integer(j_mm_count)) {
			// TODO
			continue;
		}

		long mm_count = json_integer_value(j_mm_count);

		for (long thismmid = 1; thismmid <= mm_count; thismmid++) {
			string thismmkey = "MM ID" + to_string(thismmid);
			json_t *j_this_mm_crap = json_object_get(j_apidata, thismmkey.c_str());

			if (!json_is_string(j_this_mm_crap)) {
				// TODO
				continue;
			}

			memset(&mmmm, 0, sizeof(struct Avalon_MM));

			char *this_mm_crap = strdupa(json_string_value(j_this_mm_crap));

			api_parse_crap(this_mm_crap, strlen(this_mm_crap)+1, &mmmm);

			// LET'S STRUGGLE!!!
			json_t *j_devid = json_object_get(j_apidata, "STATS");

			if (!json_is_integer(j_devid))
				continue;

			sqlite3_prepare_v2(thisdb, crap_stmt.c_str(), -1, &stmt, NULL);

			bi(1, StartTime);
			bb(2, Remote_Addr, Remote_AddrLen);
			bi(3, Remote_Port);

			bi(4, json_integer_value(j_devid));
			bi(5, thismmid);

			// Ver ~ Elapsed
			bt(6, mmmm.Ver);
			bb(7, &mmmm.DNA, 8);
			bi(8, mmmm.Elapsed);

			int di, li;

			// MW
			di = 9;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.MW[li]);

			bi(13, mmmm.LW);

			di = 14;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.MH[li]);

			bi(18, mmmm.HW);
			bd(19, mmmm.DH);
			bd(20, mmmm.Temp);
			bd(21, mmmm.TMax);
			bi(22, mmmm.Fan);
			bi(23, mmmm.FanR);

			di = 24;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.Vi[li]);

			di = 28;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.Vo[li]);

			di = 32;

			int li2;

			for (li=0; li<4; li++)
				for (li2=0; li2<6; li2++)
					bi(di+li*6+li2, mmmm.PLL[li][li2]);

			bd(56, mmmm.GHSmm);
			bd(57, mmmm.WU);
			bd(58, mmmm.Freq);
			bi(59, mmmm.PG);
			bi(60, mmmm.Led);

			di = 134;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.ECHU[li]);

			bi(133, mmmm.TA);

			bi(138, mmmm.ECMM);

			bi(669, mmmm.FM);
			di = 670;
			for (li=0; li<4; li++)
				bi(di+li, mmmm.CRC[li]);

//			for (li=0; li<3; li++)
//				for (li2=0; li2<6; li2++)
//					bi(di+li*6+li2, mmmm.PLL[li][li2]);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);

		}


	}

}

vector<string> CgMinerAPIProcessor::Crap_LineBurster(string linestr) {
	stringstream crap_sss(linestr);
	vector<string> line_entries;

	while (crap_sss.good()) {
		string substr;
		getline(crap_sss, substr, ' ');
		if (substr.c_str()[0] > 0x20 )
			line_entries.push_back(substr);
	}

	return line_entries;
}

const char *CgMinerAPIProcessor::APITypeString(CgMinerAPIProcessor::CgMiner_APIType v) {
	switch (v) {
		case Summary:
			return "summary";
		case EStats:
			return "estats";
		case EDevs:
			return "edevs";
		case Pools:
			return "pools";
		default:
			return "喵喵喵？？";
	}
}

