//
// Created by root on 17-2-15.
//

#include "../amsd.hpp"

static const string api_cmd_summary = "{\"command\":\"summary\"}";
static const string api_cmd_estats = "{\"command\":\"estats\"}";
static const string api_cmd_edevs = "{\"command\":\"edevs\"}";
static const string api_cmd_pools = "{\"command\":\"pools\"}";

time_t last_collect_time = 0;

shared_timed_mutex lock_datacollector;

map<ReimuInetAddr, Avalon_Controller> Controllers;

uint amsd_datacollection_interval = 120;

static void event_cb(struct bufferevent *bev, short events, void *ptr){
	CgMinerAPIProcessor *apibuf = (CgMinerAPIProcessor *)ptr;

	if (events & BEV_EVENT_CONNECTED) {
		fprintf(stderr,"amsd: datacollector: %s (%p) connected\n", apibuf->Remote_AddrText.c_str(), bev);
		const string *cmdstr = &api_cmd_summary;

		if (!apibuf->CmdWritten) {
			switch (apibuf->APIType) {
				case CgMinerAPIProcessor::Summary:
					break;
				case CgMinerAPIProcessor::EStats:
					cmdstr = &api_cmd_estats;
					break;
				case CgMinerAPIProcessor::EDevs:
					cmdstr = &api_cmd_edevs;
					break;
				case CgMinerAPIProcessor::Pools:
					cmdstr = &api_cmd_pools;
					break;
			}

			bufferevent_write(bev, cmdstr->c_str(), cmdstr->length());
			bufferevent_write(bev, "\n", 1);
			apibuf->CmdWritten = 1;
		}

	} else {
		if (events & BEV_EVENT_EOF) {
//			fprintf(stderr, "amsd: datacollector: %s %p done\n", apibuf->AddrText.c_str(), bev);
			unsigned char *s = &apibuf->NetIOBuf[0];
			apibuf->NetIOBuf.push_back(0);
			apibuf->Process();
		} else if (events & BEV_EVENT_ERROR) {

		}

		fprintf(stderr, "amsd: datacollector: %s (%p) connection %s (%d), %zu bytes received\n", apibuf->Remote_AddrText.c_str(),
			bev, (events & BEV_EVENT_EOF) ? "done" : "error", events, apibuf->NetIOBuf.size()-1);

		bufferevent_free(bev);
		delete apibuf;
	}

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

	last_collect_time = time(NULL);

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

			if (bufferevent_socket_connect(bebuf, (struct sockaddr *)remote_sockaddr, sockaddr_len) < 0) {
				bufferevent_free(bebuf);
				continue;
			} else {
				abuf = new CgMinerAPIProcessor((CgMinerAPIProcessor::CgMiner_APIType)apicat, last_collect_time, remote_inaddr, (size_t)inaddr_len, remote_port);
				bufferevent_setcb(bebuf, conn_readcb, conn_writecb, event_cb, abuf);
				bufferevent_enable(bebuf, EV_READ | EV_WRITE);
			}
		}

	}

	sqlite3_finalize(stmt);

	db_close(thisdb);

	event_base_dispatch(eventbase);
	event_base_free(eventbase);
}

static void *amsd_datacollector_thread(void *meow){
	timeval tv_begin, tv_end, tv_diff;

	while (1) {
		if (!amsd_datacollection_interval)
			pthread_exit(NULL);

		lock_datacollector.lock();
		fprintf(stderr, "amsd: datacollector: collector instance started\n");
		gettimeofday(&tv_begin, NULL);
		amsd_datacollector_instance();
		gettimeofday(&tv_end, NULL);
		timersub(&tv_end, &tv_begin, &tv_diff);
		lock_datacollector.unlock();
		fprintf(stderr, "amsd: datacollector: collector instance done (%zu.%zu secs), sleeping %u secs...\n", tv_diff.tv_sec,
			tv_diff.tv_usec, amsd_datacollection_interval);
		sleep(amsd_datacollection_interval);
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



#define bi(n,a)		sqlite3_bind_int64(stmt, n, a)
#define bd(n,d)		sqlite3_bind_double(stmt, n, d)
#define bt(n,s)		sqlite3_bind_text(stmt, n, s, -1, SQLITE_STATIC)
#define bb(n,b,s)	sqlite3_bind_blob64(stmt, n, b, s, SQLITE_STATIC)

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


CgMinerAPIProcessor::CgMinerAPIProcessor(CgMinerAPIProcessor::CgMiner_APIType t, time_t tm, const void *addr, size_t addrlen, uint16_t port) {
	APIType = t;
	StartTime = tm;
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
		db_open(dbpath_summary, thisdb);
			ProcessData("SUMMARY", thisdb, aq_summary);
			db_close(thisdb);
			break;
		case EStats:
			ProcessHolyShittyCrap(); // The API output is NOT easy to parse nor easy to read!!!
			break;
		case EDevs:
		db_open(dbpath_device, thisdb);
			ProcessData("DEVS", thisdb, aq_device);
			db_close(thisdb);
			break;
		case Pools:
		db_open(dbpath_pool, thisdb);
			ProcessData("POOLS", thisdb, aq_pool);
			db_close(thisdb);
			break;
	}

}


void CgMinerAPIProcessor::ProcessData(const char *api_obj_name, sqlite3 *db, CgMinerAPIQueryAutomator aq) {
	json_t *j_apidata_array = json_object_get(j_apidata_root, api_obj_name);
	const char *sql_stmt = aq.GetInsertStmt().c_str();
	vector<string> jentries = aq.GetJsonKeys();

	sqlite3_stmt *stmt;
	int narg;

	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}

	json_t *j_apidata;

	size_t j;

	json_array_foreach(j_apidata_array, j, j_apidata) {

		if (!json_is_object(j_apidata)) {
			// TODO
			return;
		}

		sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, NULL);

		if (!stmt)
			return;

		bi(1, StartTime);
		bb(2, Remote_Addr,Remote_AddrLen);
		bi(3, Remote_Port);

		narg = 4;

		for (auto const &key: jentries) {
			json_t *buf0 = json_object_get(j_apidata, key.c_str());

			if (!buf0) {
				// TODO
				return;
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

}

void CgMinerAPIProcessor::ProcessHolyShittyCrap() { // Special function designed to get rid of that shitty API output
	// We are not using regex because it is too SLOW.
	json_t *j_apidata_array = json_object_get(j_apidata_root, "STATS");
	sqlite3 *thisdb;
	sqlite3_stmt *stmt;
	Avalon_MM mmmm;


	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}

	db_open(dbpath_module_avalon7, thisdb);

	if (!thisdb) {
		// TODO
		return;
	}

	json_t *j_apidata;
	size_t j;

	sqlite3_exec(thisdb, "BEGIN", NULL, NULL, NULL);

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

//			for (li=0; li<3; li++)
//				for (li2=0; li2<6; li2++)
//					bi(di+li*6+li2, mmmm.PLL[li][li2]);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);

		}


	}

	sqlite3_exec(thisdb, "COMMIT", NULL, NULL, NULL);

	db_close(thisdb);

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

