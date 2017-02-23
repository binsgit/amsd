//
// Created by root on 17-2-16.
//

#include "amsd.hpp"

#define spair		pair<sqlite3*, string>

sqlite3 *db_controller = NULL;
sqlite3 *db_mod_policy = NULL;
sqlite3 *db_summary = NULL;
sqlite3 *db_pool = NULL;
sqlite3 *db_module = NULL;

map<string, pair<sqlite3*, string>> db = {
	{"controller", spair(NULL, "(Time UNSIGNED INT64, Addr BLOB(8), Port UNSIGNED INT16)")},
	{"mod_policy", spair(NULL, "(Time UNSIGNED INT64, DNA BLOB(8), CriticalValue_Hashrate UNSIGNED INT32)")},
	{"summary", spair(NULL, "(Time UNSIGNED INT64, Addr BLOB(8), Port UNSIGNED INT16, "
		"Elapsed INT, MHSav REAL, MHS5s REAL, MHS1m REAL, MHS5m REAL, MHS15m REAL, "
		"FoundBlocks INT, Getworks INT, Accepted INT, Rejected INT, HardwareErrors INT, "
		"Utility REAL, Discarded INT, Stale INT, GetFailures INT, LocalWork INT, RemoteFailures INT, "
		"NetworkBlocks INT, TotalMH REAL, WorkUtility REAL, "
		"DifficultyAccepted REAL, DifficultyRejected REAL, DifficultyStale REAL, "
		"BestShare INT, DeviceHardware REAL, DeviceRejected REAL, PoolRejected REAL, PoolStale REAL, "
		"Lastgetwork INT)")},
	{"pool", spair(NULL, "(Time UNSIGNED INT64, Addr BLOB(8), Port UNSIGNED INT16, "
		"PoolID INT, URL TEXT, Status INT, Priority INT, Quota INT, LongPoll INT, "
		"Getworks INT, Accepted INT, Rejected INT, Works INT, Discarded INT, "
		"Stale INT, GetFailures INT, RemoteFailures INT, User TEXT, "
		"LastShareTime INT64, Diff1Shares INT, ProxyType TEXT, Proxy TEXT, "
		"DifficultyAccepted REAL, DifficultyRejected REAL, DifficultyStale REAL, "
		"LastShareDifficulty REAL, WorkDifficulty REAL, HasStratum INT, StratumURL TEXT, "
		"StratumDifficulty REAL, HasGBT INT, BestShare INT, PoolRejected REAL, PoolStale REAL, "
		"BadWork INT, CurrentBlockHeight INT, CurrentBlockVersion INT)")},
	{"module", spair(NULL, "(Time UNSIGNED INT64, Addr BLOB(8), Port UNSIGNED INT16, DeviceID INT, ModuleID INT, "
		"Ver TEXT, DNA BLOB(8), Elapsed INT, MW_0 INT, MW_1 INT, MW_2 INT, MW_3 INT, LW INT, "
		"MH_0 INT, MH_1 INT, MH_2 INT, MH_3 INT, HW INT, DH REAL, Temp INT, TMax INT, Fan INT, FanR INT, "
		"Vi_0 INT, Vi_1 INT, Vi_2 INT, Vi_3 INT, Vo_0 INT, Vo_1 INT, Vo_2 INT, Vo_3 INT, "
		"PLL_0_0 INT, PLL_0_1 INT, PLL_0_2 INT, PLL_0_3 INT, PLL_0_4 INT, PLL_0_5 INT, "
		"PLL_1_0 INT, PLL_1_1 INT, PLL_1_2 INT, PLL_1_3 INT, PLL_1_4 INT, PLL_1_5 INT, "
		"PLL_2_0 INT, PLL_2_1 INT, PLL_2_2 INT, PLL_2_3 INT, PLL_2_4 INT, PLL_2_5 INT, "
		"PLL_3_0 INT, PLL_3_1 INT, PLL_3_2 INT, PLL_3_3 INT, PLL_3_4 INT, PLL_3_5 INT, "
		"GHSmm REAL, WU REAL, Freq REAL, PG INT, Led INT, "
		"MW_0_0 INT, MW_0_1 INT, MW_0_2 INT, MW_0_3 INT, MW_0_4 INT, MW_0_5 INT, MW_0_6 INT, MW_0_7 INT, "
		"MW_0_8 INT, MW_0_9 INT, MW_0_10 INT, MW_0_11 INT, MW_0_12 INT, MW_0_13 INT, MW_0_14 INT, MW_0_15 INT, "
		"MW_0_16 INT, MW_0_17 INT, "
		"MW_1_0 INT, MW_1_1 INT, MW_1_2 INT, MW_1_3 INT, MW_1_4 INT, MW_1_5 INT, MW_1_6 INT, MW_1_7 INT, "
		"MW_1_8 INT, MW_1_9 INT, MW_1_10 INT, MW_1_11 INT, MW_1_12 INT, MW_1_13 INT, MW_1_14 INT, MW_1_15 INT, "
		"MW_1_16 INT, MW_1_17 INT, "
		"MW_2_0 INT, MW_2_1 INT, MW_2_2 INT, MW_2_3 INT, MW_2_4 INT, MW_2_5 INT, MW_2_6 INT, MW_2_7 INT, "
		"MW_2_8 INT, MW_2_9 INT, MW_2_10 INT, MW_2_11 INT, MW_2_12 INT, MW_2_13 INT, MW_2_14 INT, MW_2_15 INT, "
		"MW_2_16 INT, MW_2_17 INT, "
		"MW_3_0 INT, MW_3_1 INT, MW_3_2 INT, MW_3_3 INT, MW_3_4 INT, MW_3_5 INT, MW_3_6 INT, MW_3_7 INT, "
		"MW_3_8 INT, MW_3_9 INT, MW_3_10 INT, MW_3_11 INT, MW_3_12 INT, MW_3_13 INT, MW_3_14 INT, MW_3_15 INT, "
		"MW_3_16 INT, MW_3_17 INT, "
		"TA INT, ECHU_0 INT, ECHU_1 INT, ECHU_2 INT, ECHU_3 INT, ECMM INT, "
		"SF_0_0 INT, SF_0_1 INT, SF_0_2 INT, SF_0_3 INT, SF_0_4 INT, SF_0_5 INT, "
		"SF_1_0 INT, SF_1_1 INT, SF_1_2 INT, SF_1_3 INT, SF_1_4 INT, SF_1_5 INT, "
		"SF_2_0 INT, SF_2_1 INT, SF_2_2 INT, SF_2_3 INT, SF_2_4 INT, SF_2_5 INT, "
		"SF_3_0 INT, SF_3_1 INT, SF_3_2 INT, SF_3_3 INT, SF_3_4 INT, SF_3_5 INT, "
		"PMUV_0 INT, PMUV_1 INT, "
		"ERATIO_0_0 REAL, ERATIO_0_1 REAL, ERATIO_0_2 REAL, ERATIO_0_3 REAL, ERATIO_0_4 REAL, ERATIO_0_5 REAL, "
		"ERATIO_0_6 REAL, ERATIO_0_7 REAL, ERATIO_0_8 REAL, ERATIO_0_9 REAL, ERATIO_0_10 REAL, "
		"ERATIO_0_11 REAL, ERATIO_0_12 REAL, ERATIO_0_13 REAL, ERATIO_0_14 REAL, ERATIO_0_15 REAL, "
		"ERATIO_0_16 REAL, ERATIO_0_17 REAL, "
		"ERATIO_1_0 REAL, ERATIO_1_1 REAL, ERATIO_1_2 REAL, ERATIO_1_3 REAL, ERATIO_1_4 REAL, ERATIO_1_5 REAL, "
		"ERATIO_1_6 REAL, ERATIO_1_7 REAL, ERATIO_1_8 REAL, ERATIO_1_9 REAL, ERATIO_1_10 REAL, "
		"ERATIO_1_11 REAL, ERATIO_1_12 REAL, ERATIO_1_13 REAL, ERATIO_1_14 REAL, ERATIO_1_15 REAL, "
		"ERATIO_1_16 REAL, ERATIO_1_17 REAL, "
		"ERATIO_2_0 REAL, ERATIO_2_1 REAL, ERATIO_2_2 REAL, ERATIO_2_3 REAL, ERATIO_2_4 REAL, ERATIO_2_5 REAL, "
		"ERATIO_2_6 REAL, ERATIO_2_7 REAL, ERATIO_2_8 REAL, ERATIO_2_9 REAL, ERATIO_2_10 REAL, "
		"ERATIO_2_11 REAL, ERATIO_2_12 REAL, ERATIO_2_13 REAL, ERATIO_2_14 REAL, ERATIO_2_15 REAL, "
		"ERATIO_2_16 REAL, ERATIO_2_17 REAL, "
		"ERATIO_3_0 REAL, ERATIO_3_1 REAL, ERATIO_3_2 REAL, ERATIO_3_3 REAL, ERATIO_3_4 REAL, ERATIO_3_5 REAL, "
		"ERATIO_3_6 REAL, ERATIO_3_7 REAL, ERATIO_3_8 REAL, ERATIO_3_9 REAL, ERATIO_3_10 REAL, "
		"ERATIO_3_11 REAL, ERATIO_3_12 REAL, ERATIO_3_13 REAL, ERATIO_3_14 REAL, ERATIO_3_15 REAL, "
		"ERATIO_3_16 REAL, ERATIO_3_17 REAL, "
		"C_0_0_0 INT, C_0_0_1 INT, C_0_0_2 INT, C_0_0_3 INT, C_0_0_4 INT, C_0_0_5 INT, C_0_0_6 INT, "
		"C_0_0_7 INT, C_0_0_8 INT, C_0_0_9 INT, C_0_0_10 INT, C_0_0_11 INT, C_0_0_12 INT, C_0_0_13 INT, "
		"C_0_0_14 INT, C_0_0_15 INT, C_0_0_16 INT, C_0_0_17 INT, C_0_1_0 INT, C_0_1_1 INT, C_0_1_2 INT, "
		"C_0_1_3 INT, C_0_1_4 INT, C_0_1_5 INT, C_0_1_6 INT, C_0_1_7 INT, C_0_1_8 INT, C_0_1_9 INT, "
		"C_0_1_10 INT, C_0_1_11 INT, C_0_1_12 INT, C_0_1_13 INT, C_0_1_14 INT, C_0_1_15 INT, C_0_1_16 INT, "
		"C_0_1_17 INT, C_0_2_0 INT, C_0_2_1 INT, C_0_2_2 INT, C_0_2_3 INT, C_0_2_4 INT, C_0_2_5 INT, "
		"C_0_2_6 INT, C_0_2_7 INT, C_0_2_8 INT, C_0_2_9 INT, C_0_2_10 INT, C_0_2_11 INT, C_0_2_12 INT, "
		"C_0_2_13 INT, C_0_2_14 INT, C_0_2_15 INT, C_0_2_16 INT, C_0_2_17 INT, C_0_3_0 INT, C_0_3_1 INT, "
		"C_0_3_2 INT, C_0_3_3 INT, C_0_3_4 INT, C_0_3_5 INT, C_0_3_6 INT, C_0_3_7 INT, C_0_3_8 INT, "
		"C_0_3_9 INT, C_0_3_10 INT, C_0_3_11 INT, C_0_3_12 INT, C_0_3_13 INT, C_0_3_14 INT, C_0_3_15 INT, "
		"C_0_3_16 INT, C_0_3_17 INT, C_0_4_0 INT, C_0_4_1 INT, C_0_4_2 INT, C_0_4_3 INT, C_0_4_4 INT, "
		"C_0_4_5 INT, C_0_4_6 INT, C_0_4_7 INT, C_0_4_8 INT, C_0_4_9 INT, C_0_4_10 INT, C_0_4_11 INT, "
		"C_0_4_12 INT, C_0_4_13 INT, C_0_4_14 INT, C_0_4_15 INT, C_0_4_16 INT, C_0_4_17 INT, C_1_0_0 INT, "
		"C_1_0_1 INT, C_1_0_2 INT, C_1_0_3 INT, C_1_0_4 INT, C_1_0_5 INT, C_1_0_6 INT, C_1_0_7 INT, "
		"C_1_0_8 INT, C_1_0_9 INT, C_1_0_10 INT, C_1_0_11 INT, C_1_0_12 INT, C_1_0_13 INT, C_1_0_14 INT, "
		"C_1_0_15 INT, C_1_0_16 INT, C_1_0_17 INT, C_1_1_0 INT, C_1_1_1 INT, C_1_1_2 INT, C_1_1_3 INT, "
		"C_1_1_4 INT, C_1_1_5 INT, C_1_1_6 INT, C_1_1_7 INT, C_1_1_8 INT, C_1_1_9 INT, C_1_1_10 INT, "
		"C_1_1_11 INT, C_1_1_12 INT, C_1_1_13 INT, C_1_1_14 INT, C_1_1_15 INT, C_1_1_16 INT, C_1_1_17 INT, "
		"C_1_2_0 INT, C_1_2_1 INT, C_1_2_2 INT, C_1_2_3 INT, C_1_2_4 INT, C_1_2_5 INT, C_1_2_6 INT, "
		"C_1_2_7 INT, C_1_2_8 INT, C_1_2_9 INT, C_1_2_10 INT, C_1_2_11 INT, C_1_2_12 INT, C_1_2_13 INT, "
		"C_1_2_14 INT, C_1_2_15 INT, C_1_2_16 INT, C_1_2_17 INT, C_1_3_0 INT, C_1_3_1 INT, C_1_3_2 INT, "
		"C_1_3_3 INT, C_1_3_4 INT, C_1_3_5 INT, C_1_3_6 INT, C_1_3_7 INT, C_1_3_8 INT, C_1_3_9 INT, "
		"C_1_3_10 INT, C_1_3_11 INT, C_1_3_12 INT, C_1_3_13 INT, C_1_3_14 INT, C_1_3_15 INT, C_1_3_16 INT, "
		"C_1_3_17 INT, C_1_4_0 INT, C_1_4_1 INT, C_1_4_2 INT, C_1_4_3 INT, C_1_4_4 INT, C_1_4_5 INT, "
		"C_1_4_6 INT, C_1_4_7 INT, C_1_4_8 INT, C_1_4_9 INT, C_1_4_10 INT, C_1_4_11 INT, C_1_4_12 INT, "
		"C_1_4_13 INT, C_1_4_14 INT, C_1_4_15 INT, C_1_4_16 INT, C_1_4_17 INT, C_2_0_0 INT, C_2_0_1 INT, "
		"C_2_0_2 INT, C_2_0_3 INT, C_2_0_4 INT, C_2_0_5 INT, C_2_0_6 INT, C_2_0_7 INT, C_2_0_8 INT, "
		"C_2_0_9 INT, C_2_0_10 INT, C_2_0_11 INT, C_2_0_12 INT, C_2_0_13 INT, C_2_0_14 INT, C_2_0_15 INT, "
		"C_2_0_16 INT, C_2_0_17 INT, C_2_1_0 INT, C_2_1_1 INT, C_2_1_2 INT, C_2_1_3 INT, C_2_1_4 INT, "
		"C_2_1_5 INT, C_2_1_6 INT, C_2_1_7 INT, C_2_1_8 INT, C_2_1_9 INT, C_2_1_10 INT, C_2_1_11 INT, "
		"C_2_1_12 INT, C_2_1_13 INT, C_2_1_14 INT, C_2_1_15 INT, C_2_1_16 INT, C_2_1_17 INT, C_2_2_0 INT, "
		"C_2_2_1 INT, C_2_2_2 INT, C_2_2_3 INT, C_2_2_4 INT, C_2_2_5 INT, C_2_2_6 INT, C_2_2_7 INT, "
		"C_2_2_8 INT, C_2_2_9 INT, C_2_2_10 INT, C_2_2_11 INT, C_2_2_12 INT, C_2_2_13 INT, C_2_2_14 INT, "
		"C_2_2_15 INT, C_2_2_16 INT, C_2_2_17 INT, C_2_3_0 INT, C_2_3_1 INT, C_2_3_2 INT, C_2_3_3 INT, "
		"C_2_3_4 INT, C_2_3_5 INT, C_2_3_6 INT, C_2_3_7 INT, C_2_3_8 INT, C_2_3_9 INT, C_2_3_10 INT, "
		"C_2_3_11 INT, C_2_3_12 INT, C_2_3_13 INT, C_2_3_14 INT, C_2_3_15 INT, C_2_3_16 INT, C_2_3_17 INT, "
		"C_2_4_0 INT, C_2_4_1 INT, C_2_4_2 INT, C_2_4_3 INT, C_2_4_4 INT, C_2_4_5 INT, C_2_4_6 INT, "
		"C_2_4_7 INT, C_2_4_8 INT, C_2_4_9 INT, C_2_4_10 INT, C_2_4_11 INT, C_2_4_12 INT, C_2_4_13 INT, "
		"C_2_4_14 INT, C_2_4_15 INT, C_2_4_16 INT, C_2_4_17 INT, C_3_0_0 INT, C_3_0_1 INT, C_3_0_2 INT, "
		"C_3_0_3 INT, C_3_0_4 INT, C_3_0_5 INT, C_3_0_6 INT, C_3_0_7 INT, C_3_0_8 INT, C_3_0_9 INT, "
		"C_3_0_10 INT, C_3_0_11 INT, C_3_0_12 INT, C_3_0_13 INT, C_3_0_14 INT, C_3_0_15 INT, C_3_0_16 INT, "
		"C_3_0_17 INT, C_3_1_0 INT, C_3_1_1 INT, C_3_1_2 INT, C_3_1_3 INT, C_3_1_4 INT, C_3_1_5 INT, "
		"C_3_1_6 INT, C_3_1_7 INT, C_3_1_8 INT, C_3_1_9 INT, C_3_1_10 INT, C_3_1_11 INT, C_3_1_12 INT, "
		"C_3_1_13 INT, C_3_1_14 INT, C_3_1_15 INT, C_3_1_16 INT, C_3_1_17 INT, C_3_2_0 INT, C_3_2_1 INT, "
		"C_3_2_2 INT, C_3_2_3 INT, C_3_2_4 INT, C_3_2_5 INT, C_3_2_6 INT, C_3_2_7 INT, C_3_2_8 INT, "
		"C_3_2_9 INT, C_3_2_10 INT, C_3_2_11 INT, C_3_2_12 INT, C_3_2_13 INT, C_3_2_14 INT, C_3_2_15 INT, "
		"C_3_2_16 INT, C_3_2_17 INT, C_3_3_0 INT, C_3_3_1 INT, C_3_3_2 INT, C_3_3_3 INT, C_3_3_4 INT, "
		"C_3_3_5 INT, C_3_3_6 INT, C_3_3_7 INT, C_3_3_8 INT, C_3_3_9 INT, C_3_3_10 INT, C_3_3_11 INT, "
		"C_3_3_12 INT, C_3_3_13 INT, C_3_3_14 INT, C_3_3_15 INT, C_3_3_16 INT, C_3_3_17 INT, C_3_4_0 INT, "
		"C_3_4_1 INT, C_3_4_2 INT, C_3_4_3 INT, C_3_4_4 INT, C_3_4_5 INT, C_3_4_6 INT, C_3_4_7 INT, "
		"C_3_4_8 INT, C_3_4_9 INT, C_3_4_10 INT, C_3_4_11 INT, C_3_4_12 INT, C_3_4_13 INT, C_3_4_14 INT, "
		"C_3_4_15 INT, C_3_4_16 INT, C_3_4_17 INT, "
		"GHSmm_0_0 REAL, GHSmm_0_1 REAL, GHSmm_0_2 REAL, GHSmm_0_3 REAL, GHSmm_0_4 REAL, GHSmm_0_5 REAL, "
		"GHSmm_0_6 REAL, GHSmm_0_7 REAL, GHSmm_0_8 REAL, GHSmm_0_9 REAL, GHSmm_0_10 REAL, GHSmm_0_11 REAL, "
		"GHSmm_0_12 REAL, GHSmm_0_13 REAL, GHSmm_0_14 REAL, GHSmm_0_15 REAL, GHSmm_0_16 REAL, GHSmm_0_17 REAL, "
		"GHSmm_1_0 REAL, GHSmm_1_1 REAL, GHSmm_1_2 REAL, GHSmm_1_3 REAL, GHSmm_1_4 REAL, GHSmm_1_5 REAL, "
		"GHSmm_1_6 REAL, GHSmm_1_7 REAL, GHSmm_1_8 REAL, GHSmm_1_9 REAL, GHSmm_1_10 REAL, GHSmm_1_11 REAL, "
		"GHSmm_1_12 REAL, GHSmm_1_13 REAL, GHSmm_1_14 REAL, GHSmm_1_15 REAL, GHSmm_1_16 REAL, GHSmm_1_17 REAL, "
		"GHSmm_2_0 REAL, GHSmm_2_1 REAL, GHSmm_2_2 REAL, GHSmm_2_3 REAL, GHSmm_2_4 REAL, GHSmm_2_5 REAL, "
		"GHSmm_2_6 REAL, GHSmm_2_7 REAL, GHSmm_2_8 REAL, GHSmm_2_9 REAL, GHSmm_2_10 REAL, GHSmm_2_11 REAL, "
		"GHSmm_2_12 REAL, GHSmm_2_13 REAL, GHSmm_2_14 REAL, GHSmm_2_15 REAL, GHSmm_2_16 REAL, GHSmm_2_17 REAL, "
		"GHSmm_3_0 REAL, GHSmm_3_1 REAL, GHSmm_3_2 REAL, GHSmm_3_3 REAL, GHSmm_3_4 REAL, GHSmm_3_5 REAL, "
		"GHSmm_3_6 REAL, GHSmm_3_7 REAL, GHSmm_3_8 REAL, GHSmm_3_9 REAL, GHSmm_3_10 REAL, GHSmm_3_11 REAL, "
		"GHSmm_3_12 REAL, GHSmm_3_13 REAL, GHSmm_3_14 REAL, GHSmm_3_15 REAL, GHSmm_3_16 REAL, GHSmm_3_17 REAL, "
		"FM INT, CRC_0 INT, CRC_1 INT, CRC_2 INT, CRC_3 INT, "
		"PVT_0_L_C INT, PVT_0_L_T INT, PVT_0_H_C INT, PVT_0_H_T INT, PVT_0_A_C INT, PVT_0_A_T INT, "
		"PVT_1_L_C INT, PVT_1_L_T INT, PVT_1_H_C INT, PVT_1_H_T INT, PVT_1_A_C INT, PVT_1_A_T INT, "
		"PVT_2_L_C INT, PVT_2_L_T INT, PVT_2_H_C INT, PVT_2_H_T INT, PVT_2_A_C INT, PVT_2_A_T INT, "
		"PVT_3_L_C INT, PVT_3_L_T INT, PVT_3_H_C INT, PVT_3_H_T INT, PVT_3_A_C INT, PVT_3_A_T INT)")},

};

int amsd_db_init(){
	int fd;
	bool inited;
	string dbpath;

	for (auto it = db.begin(); it != db.end(); ++it) {
		dbpath = path_runtime + it->first + ".db";

		fd = open(dbpath.c_str(), O_RDWR);

		if (fd < 1) {
			inited = 0;
			if (errno != ENOENT)
				return -1;
		} else {
			inited = 1;
			close(fd);
		}

		if (sqlite3_open_v2(dbpath.c_str(), &it->second.first,
				    SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
				    NULL)) {
			fprintf(stderr, "amsd: database: failed to open database %s: %s\n", it->first.c_str(),
				sqlite3_errmsg(it->second.first));
			return -1;
		}


		if (!inited) {
			string stmt = "PRAGMA journal_mode=WAL; CREATE TABLE " + it->first + " " + it->second.second;
			fprintf(stderr, "amsd: database: initializing database %s\n", it->first.c_str());
			if (sqlite3_exec(it->second.first, stmt.c_str(), NULL, NULL, NULL)) {
				fprintf(stderr, "amsd: database: failed to initialize database %s: %s\n", it->first.c_str(),
					sqlite3_errmsg(it->second.first));
				return -1;
			}
		}

		fprintf(stderr, "amsd: database: opened database %s\n", it->first.c_str());
	}

	db_controller = db["controller"].first;
	db_mod_policy = db["mod_policy"].first;
	db_summary = db["summary"].first;
	db_module = db["module"].first;
	db_pool = db["pool"].first;

	return 0;

}