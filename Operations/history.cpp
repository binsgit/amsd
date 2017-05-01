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

struct pCtx_i {
    time_t LastDataCollection;
    pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;

    map<int64_t, int64_t> *times_elapsed;

    vector<time_t> *TimeTemp;
    vector<string> *URLTemp;
    vector<double> *SDATemp;

    map<string, map<int64_t, double>> *sorter;
};

static void *getSummary(void *userp){
	pCtx_i *thisCtx = (pCtx_i *)userp;

	try {
		SQLAutomator::SQLite3 *thisdb = db_summary.OpenSQLite3();

		thisdb->Prepare("SELECT Time, SUM(Elapsed) FROM summary WHERE Time >= ?1 GROUP BY Time");
		thisdb->Bind(1, thisCtx->LastDataCollection-864000);

		while (thisdb->Step() == SQLITE_ROW) {
			int64_t thistime = thisdb->Column(0);
			int64_t thiselapsed = thisdb->Column(1);

			thisCtx->times_elapsed->insert(pair<int64_t, int64_t>(thistime, thiselapsed));
		}

		delete thisdb;
	} catch (Reimu::Exception e) {


	}

	pthread_exit(NULL);

}

static void *getPool(void *userp) {
	pCtx_i *thisCtx = (pCtx_i *) userp;

	try {
		SQLAutomator::SQLite3 *thisdb = db_pool.OpenSQLite3();

		thisdb->Prepare("SELECT Time, URL, SUM(DifficultyAccepted) FROM pool WHERE Time >= ?1 GROUP BY Time, URL");
		thisdb->Bind(1, thisCtx->LastDataCollection - 864000);

		while (thisdb->Step() == SQLITE_ROW) {
			thisCtx->TimeTemp->push_back((int64_t)thisdb->Column(0));
			thisCtx->URLTemp->push_back(thisdb->Column(1).operator std::string());
			thisCtx->SDATemp->push_back((double)thisdb->Column(2));
		}

		delete thisdb;
	} catch (Reimu::Exception e) {


	}

	pthread_exit(NULL);

}

int AMSD::Operations::history(json_t *in_data, json_t *&out_data){

	json_t *j_type = json_object_get(in_data, "type");
	json_t *j_timearray, *j_valuearray, *j_value2array;
	json_t *j_objtmp;

	if (!json_is_string(j_type))
		return -1;

	string type(json_string_value(j_type));

	if (type == "hashrate") {

		struct pCtx_i *thisCtx = (struct pCtx_i *)malloc(sizeof(struct pCtx_i));
		thisCtx->times_elapsed = new map<int64_t, int64_t>();
		thisCtx->SDATemp = new vector<double>();
		thisCtx->sorter = new map<string, map<int64_t, double>>;
		thisCtx->URLTemp = new vector<string>;
		thisCtx->TimeTemp = new vector<time_t>;

		thisCtx->LastDataCollection = RuntimeData::TimeStamp::LastDataCollection();

		pthread_t t_gs, t_gp;

		pthread_create(&t_gs, NULL, &getSummary, thisCtx);
		pthread_create(&t_gp, NULL, &getPool, thisCtx);


		j_value2array = json_array();

		pthread_join(t_gs, NULL);

		for (auto const &it_t: *thisCtx->times_elapsed) {
			json_array_append_new(j_value2array, json_integer(it_t.first));
		}

		json_object_set_new(out_data, "times", j_value2array);

		j_value2array = json_array();

		pthread_join(t_gp, NULL);

		for (int j=0; j<thisCtx->TimeTemp->size(); j++) {
			time_t thistime = thisCtx->TimeTemp->operator[](j);

			thisCtx->sorter->operator[](thisCtx->URLTemp->operator[](j))[thistime] =
				diffaccept2ghs(thisCtx->SDATemp->operator[](j), (size_t)thisCtx->times_elapsed->operator[](thistime));
		}

		for (auto &it_s: *thisCtx->sorter) {

			j_objtmp = json_object();

			for (auto &it_t: *thisCtx->times_elapsed) {
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

		delete thisCtx->sorter;
		delete thisCtx->TimeTemp;
		delete thisCtx->SDATemp;
		delete thisCtx->URLTemp;
		delete thisCtx->times_elapsed;

		free(thisCtx);

	} else if (type == "aliverate") {

		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();


		thisdb->Prepare("SELECT Time, Count(DISTINCT(Addr)), Count(ModuleID) FROM module_avalon7 WHERE Time >= ?1 GROUP BY Time");

		thisdb->Bind(1, RuntimeData::TimeStamp::LastDataCollection()-864000);

		j_timearray = json_array();
		j_valuearray = json_array();
		j_value2array = json_array();

		while (thisdb->Step() == SQLITE_ROW) {
			json_array_append_new(j_timearray, json_integer(thisdb->Column(0)));
			json_array_append_new(j_valuearray, json_integer(thisdb->Column(1)));
			json_array_append_new(j_value2array, json_integer(thisdb->Column(2)));
		}

		json_object_set_new(out_data, "times", j_timearray);
		json_object_set_new(out_data, "ctls", j_valuearray);
		json_object_set_new(out_data, "mods", j_value2array);

		delete thisdb;

	} else {
		return -1;
	}

	return 0;
}