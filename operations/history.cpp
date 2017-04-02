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

int AMSD::Operations::history(json_t *in_data, json_t *&out_data){
	NoLoginReq_Flag;
	json_t *j_type = json_object_get(in_data, "type");
	json_t *j_timearray, *j_valuearray, *j_value2array;
	json_t *j_objtmp;

	sqlite3 *thisdb, *thisdb1;
	sqlite3_stmt *stmt, *stmt1;

	if (!json_is_string(j_type))
		return -1;

	string type(json_string_value(j_type));

	if (type == "hashrate") {
		map<int64_t, int64_t> times_elapsed;
		map<string, map<int64_t, double>> sorter1;
		int64_t thistime, thiselapsed;

		db_open(db_pool.DatabaseURI.c_str(), thisdb);
		db_open(db_summary.DatabaseURI.c_str(), thisdb1);

		sqlite3_prepare_v2(thisdb1, "SELECT Time, SUM(Elapsed) FROM summary "
			"WHERE Time >= ?1 GROUP BY Time", -1, &stmt1, NULL);

		sqlite3_prepare(thisdb, "SELECT Time, URL, SUM(DifficultyAccepted) FROM pool "
			"WHERE Time >= ?1 GROUP BY Time, URL", -1, &stmt, NULL);

		sqlite3_bind_int64(stmt, 1, RuntimeData::TimeStamp::LastDataCollection()-864000);
		sqlite3_bind_int64(stmt1, 1, RuntimeData::TimeStamp::LastDataCollection()-864000);

		while (sqlite3_step(stmt1) == SQLITE_ROW) {
			thistime = sqlite3_column_int64(stmt1, 0);
			thiselapsed = sqlite3_column_int64(stmt1, 1);
//			fprintf(stderr,"aaaa: %" PRIu64 " %" PRIu64 "\n", thistime, thiselapsed);
			times_elapsed[thistime] = thiselapsed;
		}

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			thistime = sqlite3_column_int64(stmt, 0);

//			fprintf(stderr,"xxxx: %" PRIu64 " %s %ld\n", thistime, sqlite3_column_text(stmt, 1), times_elapsed[thistime]);
			sorter1[string((char *)sqlite3_column_text(stmt, 1))]
			[thistime] =
				diffaccept2ghs(sqlite3_column_double(stmt, 2), (size_t)times_elapsed[thistime]);
		}

		sqlite3_finalize(stmt);
		sqlite3_finalize(stmt1);

		db_close(thisdb);
		db_close(thisdb1);


		j_value2array = json_array();

		for (auto const &it_t: times_elapsed) {
			json_array_append_new(j_value2array, json_integer(it_t.first));
		}

		json_object_set_new(out_data, "times", j_value2array);

		j_value2array = json_array();

		for (auto &it_s: sorter1) {

			j_objtmp = json_object();

			for (auto &it_t: times_elapsed) {
				it_s.second.insert(pair<int64_t, double>((int64_t)it_t.first, (double)0));
			}

			j_valuearray = json_array();

			for (auto const &it_p: it_s.second) {
				json_array_append_new(j_valuearray, json_real(it_p.second));
			}


			json_object_set_new(j_objtmp, "url", json_string(it_s.first.c_str()));
			json_object_set_new(j_objtmp, "hashrates", j_valuearray);

			json_array_append_new(j_value2array, j_objtmp);
		}

		json_object_set_new(out_data, "mdzz", j_value2array);

	} else if (type == "aliverate") {
		db_open(db_module_avalon7.DatabaseURI.c_str(), thisdb);
		sqlite3_prepare(thisdb, "SELECT Time, Count(DISTINCT(Addr)), Count(ModuleID) FROM module_avalon7 "
			"WHERE Time >= ?1 GROUP BY Time", -1, &stmt, NULL);

		sqlite3_bind_int64(stmt, 1, RuntimeData::TimeStamp::LastDataCollection()-864000);

		j_timearray = json_array();
		j_valuearray = json_array();
		j_value2array = json_array();

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			json_array_append_new(j_timearray, json_integer(sqlite3_column_int64(stmt, 0)));
			json_array_append_new(j_valuearray, json_integer(sqlite3_column_int64(stmt, 1)));
			json_array_append_new(j_value2array, json_integer(sqlite3_column_int64(stmt, 2)));
		}

		sqlite3_finalize(stmt);
		db_close(thisdb);

		json_object_set_new(out_data, "times", j_timearray);
		json_object_set_new(out_data, "ctls", j_valuearray);
		json_object_set_new(out_data, "mods", j_value2array);

	} else {
		return -1;
	}

	return 0;
}