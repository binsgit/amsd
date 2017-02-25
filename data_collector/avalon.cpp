//
// Created by root on 17-2-23.
//

#include <jansson.h>
#include "../amsd.hpp"

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

	fprintf(stderr, "amsd: caqa: %s\n", InsertStmt.c_str());

	return InsertStmt;
}


CgMinerAPIProcessor::CgMinerAPIProcessor(CgMinerAPIProcessor::CgMiner_APIType t, time_t tm, const void *addr, size_t addrlen, uint16_t port) {

	Type = t;
	Time = tm;
	AddrLen = addrlen;
	memcpy(Addr, addr, addrlen);
	Port = port;
}

void CgMinerAPIProcessor::Process() {
	fprintf(stderr, "amsd: CAP: process: type %d at %p\n", Type, this);

	unsigned char *apidata = &Buf[0];

	json_error_t jerror;

	j_apidata_root = json_loads((const char *)apidata, 0, &jerror);

	if (!j_apidata_root) {
		// TODO
		fprintf(stderr, "amsd: CAP: process: json error at %p: line %d: %s\n", this, jerror.line, jerror.text);
		return;
	}

	WriteDatabase();

	json_decref(j_apidata_root);

}


void CgMinerAPIProcessor::WriteDatabase() {
	sqlite3 *thisdb;

	switch (Type) {
		case Summary:
			db_open(dbpath_summary, thisdb);
			ProcessData("SUMMARY", thisdb, aq_summary);
			db_close(thisdb);
			break;
		case EStats:
			cerr << "WARNING: Processing CRAP!\n";
			ProcessHolyShittyCrap();
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

		bi(1, Time);
		bb(2, Addr,AddrLen);
		bi(3, Port);

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

void CgMinerAPIProcessor::ProcessHolyShittyCrap() {
	json_t *j_apidata_array = json_object_get(j_apidata_root, "STATS");
	sqlite3 *thisdb;
	sqlite3_stmt *stmt;
	std::vector<string> crap_line;
	Avalon_MM mmmm;


	if (!json_is_array(j_apidata_array)) {
		// TODO
		return;
	}

	db_open(dbpath_module, thisdb);

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

			bi(1, Time);
			bb(2, Addr, AddrLen);
			bi(3, Port);

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

