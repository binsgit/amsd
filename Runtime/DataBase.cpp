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

Reimu::SQLAutomator db_controller, db_user, db_issue, db_summary, db_pool, db_device,
	db_module_policy, db_module_avalon7;

int AMSD::Database::Init() {
	db_controller.TableName = "controller";
	db_controller.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_controller.TableName + ".db";
	db_controller.Statement_Ext(CREATE_TABLE, NULL, "UNIQUE(Addr, Port) ON CONFLICT IGNORE");
	db_controller.InsertColumns({{"Time", INTEGER},
				     {"Addr", BLOB},
				     {"Port", INTEGER},
				     {"Type", INTEGER}});

	db_user.TableName = "user";
	db_user.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_user.TableName + ".db";
	db_user.Statement_Ext(CREATE_TABLE, NULL, "UNIQUE(UserName) ON CONFLICT ABORT");
	db_user.InsertColumns({{"UID", INTEGER|PRIMARY_KEY|AUTOINCREMENT|NOT_NULL},
			       {"UserType", INTEGER},
			       {"UserName", TEXT},
			       {"NickName", TEXT},
			       {"Password", BLOB},
			       {"Token", TEXT},
			       {"CTime", INTEGER},
			       {"MTime", INTEGER},
			       {"ATime", INTEGER},
			       {"Avatar", BLOB},
			       {"ExtData", BLOB}});


	db_issue.TableName = "issue";
	db_issue.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_issue.TableName + ".db";
	db_issue.InsertColumns({{"Time", INTEGER},
				{"Addr", BLOB},
				{"Port", INTEGER},
				{"Type", INTEGER},
				{"Issue", BLOB}});

	db_module_policy.TableName = "module_policy";
	db_module_policy.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_module_policy.TableName + ".db";
	db_module_policy.InsertColumns({{"MTime", INTEGER},
				{"DNA", BLOB},
				{"Policy", BLOB}});

	db_summary.TableName = "summary";
	db_summary.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_summary.TableName + ".db";
	db_summary.InsertColumns({{"Time", INTEGER},
				  {"Addr", BLOB},
				  {"Port", INTEGER},
				  {"Elapsed", INTEGER},
				  {"MHSav", REAL},
				  {"MHS5s", REAL},
				  {"MHS1m", REAL},
				  {"MHS5m", REAL},
				  {"MHS15m", REAL},
				  {"FoundBlocks", INTEGER},
				  {"Getworks", INTEGER},
				  {"Accepted", INTEGER},
				  {"Rejected", INTEGER},
				  {"HardwareErrors", INTEGER},
				  {"Utility", REAL},
				  {"Discarded", INTEGER},
				  {"Stale", INTEGER},
				  {"GetFailures", INTEGER},
				  {"LocalWork", INTEGER},
				  {"RemoteFailures", INTEGER},
				  {"NetworkBlocks", INTEGER},
				  {"TotalMH", REAL},
				  {"WorkUtility", REAL},
				  {"DifficultyAccepted", REAL},
				  {"DifficultyRejected", REAL},
				  {"DifficultyStale", REAL},
				  {"BestShare", INTEGER},
				  {"DeviceHardware", REAL},
				  {"DeviceRejected", REAL},
				  {"PoolRejected", REAL},
				  {"PoolStale", REAL},
				  {"Lastgetwork", INTEGER}});


	db_pool.TableName = "pool";
	db_pool.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_pool.TableName + ".db";
	db_pool.InsertColumns({{"Time", INTEGER},
			       {"Addr", BLOB},
			       {"Port", INTEGER},
			       {"PoolID", INTEGER},
			       {"URL", TEXT},
			       {"Status", INTEGER},
			       {"Priority", INTEGER},
			       {"Quota", INTEGER},
			       {"LongPoll", INTEGER},
			       {"Getworks", INTEGER},
			       {"Accepted", INTEGER},
			       {"Rejected", INTEGER},
			       {"Works", INTEGER},
			       {"Discarded", INTEGER},
			       {"Stale", INTEGER},
			       {"GetFailures", INTEGER},
			       {"RemoteFailures", INTEGER},
			       {"User", TEXT},
			       {"LastShareTime", INTEGER},
			       {"Diff1Shares", INTEGER},
			       {"ProxyType", TEXT},
			       {"Proxy", TEXT},
			       {"DifficultyAccepted", REAL},
			       {"DifficultyRejected", REAL},
			       {"DifficultyStale", REAL},
			       {"LastShareDifficulty", REAL},
			       {"WorkDifficulty", REAL},
			       {"HasStratum", INTEGER},
			       {"StratumActive", INTEGER},
			       {"StratumURL", TEXT},
			       {"StratumDifficulty", REAL},
			       {"HasGBT", INTEGER},
			       {"BestShare", INTEGER},
			       {"PoolRejected", REAL},
			       {"PoolStale", REAL},
			       {"BadWork", INTEGER},
			       {"CurrentBlockHeight", INTEGER},
			       {"CurrentBlockVersion", INTEGER}});


	db_device.TableName = "device";
	db_device.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_device.TableName + ".db";
	db_device.InsertColumns({{"Time", INTEGER},
				 {"Addr", BLOB},
				 {"Port", INTEGER},
				 {"ASC", INTEGER},
				 {"Name", TEXT},
				 {"ID", INTEGER},
				 {"Enabled", TEXT},
				 {"Status", TEXT},
				 {"Temperature", REAL},
				 {"MHSav", REAL},
				 {"MHS5s", REAL},
				 {"MHS1m", REAL},
				 {"MHS5m", REAL},
				 {"MHS15m", REAL},
				 {"Accepted", INTEGER},
				 {"Rejected", INTEGER},
				 {"HardwareErrors", INTEGER},
				 {"Utility", REAL},
				 {"LastSharePool", INTEGER},
				 {"LastShareTime", INTEGER},
				 {"TotalMH", REAL},
				 {"Diff1Work", INTEGER},
				 {"DifficultyAccepted", REAL},
				 {"DifficultyRejected", REAL},
				 {"LastShareDifficulty", REAL},
				 {"NoDevice", INTEGER},
				 {"LastValidWork", INTEGER},
				 {"DeviceHardware", REAL},
				 {"DeviceRejected", REAL},
				 {"DeviceElapsed", INTEGER}});

	db_module_avalon7.TableName = "module_avalon7";
	db_module_avalon7.DatabaseURI = AMSD::Config::Path_RuntimeDir + db_module_avalon7.TableName + ".db";
	db_module_avalon7.InsertColumns({{"Time", INTEGER},
					 {"Addr", BLOB},
					 {"Port", INTEGER},
					 {"DeviceID", INTEGER},
					 {"ModuleID", INTEGER},
					 {"Ver", TEXT},
					 {"DNA", BLOB},
					 {"Elapsed", INTEGER}});

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"MW_"+to_string(j), INTEGER});
	}

	db_module_avalon7.InsertColumn({"LW", INTEGER});

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"MH_"+to_string(j), INTEGER});
	}

	db_module_avalon7.InsertColumns({{"HW", INTEGER},
					 {"DH", REAL},
					 {"Temp", INTEGER},
					 {"TMax", INTEGER},
					 {"Fan", INTEGER},
					 {"FanR", INTEGER}});

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"Vi_"+to_string(j), INTEGER});
	}

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"Vo_"+to_string(j), INTEGER});
	}

	for (int j=0; j<4; j++) {
		for (int k=0; k<6; k++) {
			db_module_avalon7.InsertColumn({"PLL_"+to_string(j)+"_"+to_string(k), INTEGER});
		}
	}

	db_module_avalon7.InsertColumns({{"GHSmm", REAL},
					 {"WU", REAL},
					 {"Freq", REAL},
					 {"PG", INTEGER},
					 {"LED", INTEGER}});

	for (int j=0; j<4; j++) {
		for (int k=0; k<18; k++) {
			db_module_avalon7.InsertColumn({"MW_"+to_string(j)+"_"+to_string(k), INTEGER});
		}
	}

	db_module_avalon7.InsertColumn({"TA", INTEGER});

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"ECHU_"+to_string(j), INTEGER});
	}

	db_module_avalon7.InsertColumn({"ECMM", INTEGER});

	for (int j=0; j<4; j++) {
		for (int k=0; k<6; k++) {
			db_module_avalon7.InsertColumn({"SF_"+to_string(j)+"_"+to_string(k), INTEGER});
		}
	}

	for (int j=0; j<2; j++) {
		db_module_avalon7.InsertColumn({"PMUV_"+to_string(j), INTEGER});
	}

	for (int j=0; j<4; j++) {
		for (int k=0; k<18; k++) {
			db_module_avalon7.InsertColumn({"ERATIO_"+to_string(j)+"_"+to_string(k), REAL});
		}
	}

	for (int j=0; j<4; j++) {
		for (int k=0; k<5; k++) {
			for (int l=0; l<18; l++) { // 可以这很强势
				db_module_avalon7.InsertColumn({"C_"+to_string(j)+"_"+to_string(k)+"_"+to_string(l), INTEGER});
			}
		}
	}

	for (int j=0; j<4; j++) {
		for (int k=0; k<18; k++) {
			db_module_avalon7.InsertColumn({"GHSmm_"+to_string(j)+"_"+to_string(k), REAL});
		}
	}

	db_module_avalon7.InsertColumn({"FM", INTEGER});

	for (int j=0; j<4; j++) {
		db_module_avalon7.InsertColumn({"CRC_"+to_string(j), INTEGER});
	}

	for (int j=0; j<4; j++) {
		for (auto const &k : {"L", "H", "A"}) {
			for (auto const &l : {"C", "T"}) {
				db_module_avalon7.InsertColumn({"PVT_"+to_string(j)+"_"+k+"_"+l, INTEGER});
			}
		}
	}



	// Prepend pragmas for all dbs

	int fd;

	for (auto *thisdb : {&db_controller, &db_user, &db_issue, &db_summary, &db_pool, &db_device,
			     &db_module_policy, &db_module_avalon7}) {
		thisdb->Statement_Ext(CREATE_TABLE, "PRAGMA journal_mode=WAL; PRAGMA page_size=4096; ", NULL);

		fd = open(thisdb->DatabaseURI.c_str(), O_RDWR);

		try {
			if (fd < 1) {
				if (errno != ENOENT)
					throw Reimu::Exception(errno);
			} else {
				close(fd);
			}


			SQLite3 sq3 = thisdb->OpenSQLite3(-1, SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE |
							  SQLITE_OPEN_CREATE, NULL);

			sq3.Exec(thisdb->Statement(CREATE_TABLE));

		} catch (Reimu::Exception e) {
			fprintf(stderr, "amsd: database: failed to open/initialize database %s: %s\n",
				thisdb->TableName.c_str(), e.ToString().c_str());
			return -1;
		}

		fprintf(stderr, "amsd: database: processed database %s\n", thisdb->TableName.c_str());

	}

	return 0;
}

