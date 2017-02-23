//
// Created by root on 17-2-23.
//

#include "../amsd.hpp"

static const char *sqlstmt_insert_summary = "INSERT INTO summary " // 32 args
	"(Time, Addr, Port, Elapsed, MHSav, MHS5s, MHS1m, MHS5m, MHS15m, FoundBlocks, Getworks, Accepted, Rejected, "
	"HardwareErrors, Utility, Discarded, Stale, GetFailures, LocalWork, RemoteFailures, "
	"NetworkBlocks, TotalMH, WorkUtility, DifficultyAccepted, DifficultyRejected, DifficultyStale, "
	"BestShare, DeviceHardware, DeviceRejected, PoolRejected, PoolStale, Lastgetwork) VALUES "
	"(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22, ?23, "
	"?24, ?25, ?26, ?27, ?28, ?29, ?30, ?31, ?32)";

static const char *sqlstmt_insert_pool = "INSERT INTO pool "
	"(Time, Addr, Port, PoolID, URL, Status, Priority, Quota, LongPoll, Getworks, Accepted, Rejected, Works, "
	"Discarded, Stale, GetFailures, RemoteFailures, User, LastShareTime64, Diff1Shares, ProxyType, Proxy, "
	"DifficultyAccepted, DifficultyRejected, DifficultyStale, LastShareDifficulty, WorkDifficulty, HasStratum, "
	"StratumURL, StratumDifficulty, HasGBT, BestShare, PoolRejected, PoolStale, BadWork, CurrentBlockHeight, "
	"CurrentBlockVersion) VALUES "
	"(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22, ?23, "
	"?24, ?25, ?26, ?27, ?28, ?29, ?30, ?31, ?32, ?33, ?34, ?35, ?36, ?37)";

static const vector<string> jentries_summary = {"Elapsed", "MHS av", "MHS 5s", "MHS 1m", "MHS 5m", "MHS 15m",
						"Found Blocks", "Getworks", "Accepted", "Rejected", "Hardware Errors",
						"Utility", "Discarded", "Stale", "Get Failures", "Local Work",
						"Remote Failures", "Network Blocks", "Total MH", "Work Utility",
						"Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
						"Best Share", "Device Hardware%", "Device Rejected%", "Pool Rejected%",
						"Pool Stale%", "Last getwork"};

static const vector<string> jentries_pool = {"POOL", "URL", "Status", "Priority", "Quota", "Long Poll", "Getworks",
					     "Accepted", "Rejected", "Works", "Discarded", "Stale", "Get Failures",
					     "Remote Failures", "User", "Last Share Time", "Diff1 Shares", "Proxy Type",
					     "Proxy", "Difficulty Accepted", "Difficulty Rejected", "Difficulty Stale",
					     "Last Share Difficulty", "Work Difficulty", "Has Stratum",
					     "Stratum Active", "Stratum URL", "Stratum Difficulty", "Has GBT",
					     "Best Share", "Pool Rejected%", "Pool Stale%", "Bad Work",
					     "Current Block Height", "Current Block Version"};


#define bi(n,a)		sqlite3_bind_int64(stmt, n, a)
#define bd(n,d)		sqlite3_bind_double(stmt, n, d)
#define bt(n,s)		sqlite3_bind_text(stmt, n, s, -1, SQLITE_STATIC)
#define bb(n,b,s)	sqlite3_bind_blob64(stmt, n, b, s, SQLITE_STATIC)



void Avalon_Controller::APIBuf(struct evbuffer *input) {

}


CgMiner_APIBuf::CgMiner_APIBuf(CgMiner_APIBuf::CgMiner_APIBuf_Type t, time_t tm, uint64_t addr, uint16_t port) {
	Type = t;
	Time = tm;
	Addr = addr;
	Port = port;
}

void CgMiner_APIBuf::Process() {
	unsigned char *apidata = &Buf[0];

	json_error_t jerror;

	j_apidata_root = json_loads((const char *)apidata, 0, &jerror);

	if (!j_apidata_root) {
		// TODO
		return;
	}

	WriteDatabase();

	json_decref(j_apidata_root);

}


void CgMiner_APIBuf::WriteDatabase() {
	switch (Type) {
		case Summary:
			ProcessData("SUMMARY", db_summary, sqlstmt_insert_summary, jentries_summary);
			break;
		case EDevs:
			break;
		case EStats:
			break;
		case Pools:
			ProcessData("POOLS", db_pool, sqlstmt_insert_pool, jentries_pool);
			break;
	}

}


void CgMiner_APIBuf::ProcessData(const char *api_obj_name, sqlite3 *db, const char *sql_stmt, const vector<string> &jentries) {
	json_t *j_apidata_array = json_object_get(j_apidata_root, api_obj_name);
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
		bb(2, &Addr, 8);
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

			narg++;
		}

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		stmt = NULL;

	}

}
