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

struct mController {
    bool Dead = false;
    string IP;
    int Port = 0;
    double GHS = 0;
    int Mods = 0;
    double TempSum = 0, TMaxSum = 0;
};

struct dtCtx {
    time_t LastDataCollection;
    pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
    map<string, mController> *ctls;
};

static void *getModules(void *pctx) {
	try {
		dtCtx *dc = (dtCtx *) pctx;

		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();

		thisdb->Prepare("SELECT Addr, Port, GHSmm, Temp, TMax FROM module_avalon7 WHERE Time = ?1");
		thisdb->Bind(1, dc->LastDataCollection);
		fprintf(stderr, "amsd: Operations::farmap::getModules: LastDataCollection: %lu\n", dc->LastDataCollection);
		mController *ctl;

		while (thisdb->Step() == SQLITE_ROW) {
			Reimu::IPEndPoint thisEP(thisdb->Column(0).operator std::pair<void *, size_t>(), thisdb->Column(1));
			fprintf(stderr, "amsd: Operations::farmap::getModules: Node %s\n", thisEP.ToString().c_str());
			pthread_mutex_lock(&dc->Lock);
			ctl = &dc->ctls->operator[](thisEP.ToString());
			pthread_mutex_unlock(&dc->Lock);

			ctl->IP = thisEP.ToString(Reimu::IPEndPoint::String_IP);
			ctl->Port = thisEP.Port;
			ctl->GHS += (double)thisdb->Column(2);
			ctl->TempSum += (double)thisdb->Column(3);
			ctl->TMaxSum += (double)thisdb->Column(4);
			ctl->Mods++;
		}

		delete thisdb;
	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

static void *getDeads(void *pctx) {
	try {
		dtCtx *dc = (dtCtx *)pctx;

		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();

		thisdb->Prepare("SELECT Addr, Port FROM issue WHERE Time = ?1 AND Type >= 0x10 AND Type < 0x20");
		thisdb->Bind(1, dc->LastDataCollection);

		mController *ctl;

		while (thisdb->Step() == SQLITE_ROW) {
			Reimu::IPEndPoint thisEP(thisdb->Column(0), (uint16_t)thisdb->ColumnBytes(0), thisdb->Column(1));

			pthread_mutex_lock(&dc->Lock);
			ctl = &dc->ctls->operator[](thisEP.ToString());
			pthread_mutex_unlock(&dc->Lock);

			ctl->Dead = 1;
		}

		delete thisdb;
	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}


int AMSD::Operations::farmap(json_t *in_data, json_t *&out_data){


	json_t *j_ctls = json_array();
	json_t *j_ctl;

	map<string, mController> ctls;

	dtCtx thisCtx;

	thisCtx.ctls = &ctls;
	thisCtx.LastDataCollection = RuntimeData::TimeStamp::LastDataCollection();

	pthread_t t_gm, t_gd;

	pthread_create(&t_gm, NULL, &getModules, &thisCtx);
	pthread_create(&t_gd, NULL, &getDeads, &thisCtx);

	pthread_join(t_gm, NULL);
	pthread_join(t_gd, NULL);

	for (auto const &it: ctls) {
		j_ctl = json_object();

		json_object_set_new(j_ctl, "IP", json_string(it.second.IP.c_str()));
		json_object_set_new(j_ctl, "Port", json_integer(it.second.Port));
		json_object_set_new(j_ctl, "GHS", json_real(it.second.GHS));
		json_object_set_new(j_ctl, "Mods", json_integer(it.second.Mods));
		json_object_set_new(j_ctl, "Temp", json_real(it.second.TempSum/it.second.Mods));
		json_object_set_new(j_ctl, "TMax", json_real(it.second.TMaxSum/it.second.Mods));
		json_object_set_new(j_ctl, "Dead", json_boolean(it.second.Dead));

		json_array_append_new(j_ctls, j_ctl);
	}

	json_object_set_new(out_data, "Controllers", j_ctls);

	return 0;
}