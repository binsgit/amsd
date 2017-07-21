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
    json_t *j_issues;
};

static void *getAvalonErrors(void *userp){
	pCtx_i *thisCtx = (pCtx_i *)userp;

	json_t *j_issue, *j_crcs, *j_echus;

	uint32_t CRC[4];
	uint32_t ECHU[4];

	try {
		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();

		thisdb->Prepare("SELECT Addr, Port, DeviceID, ModuleID, CRC_0, CRC_1, CRC_2, CRC_3, "
				       "ECHU_0, ECHU_1, ECHU_2, ECHU_3, Ver, WU, DH, DNA "
				       "FROM module_avalon7 WHERE Time = ?1 AND "
				       "(ECHU_0 > 0 OR ECHU_1 > 0 OR ECHU_2 > 0 OR ECHU_3 > 0)");

		thisdb->Bind(1, thisCtx->LastDataCollection);


		while (thisdb->Step() == SQLITE_ROW) {
			j_issue = json_object();

			json_object_set_new(j_issue, "type", json_integer(DataProcessing::Issue::IssueType::AvalonError));

			vector<uint8_t> thisaddr = thisdb->Column(0);
			std::pair<void *, size_t> thisaddrp(thisaddr.data(), thisaddr.size());

			Reimu::IPEndPoint remoteEP(thisaddrp, thisdb->Column(1));

			json_object_set_new(j_issue, "ip", json_string(remoteEP.ToString(Reimu::IPEndPoint::String_IP).c_str()));
			json_object_set_new(j_issue, "port", json_integer(remoteEP.Port));
			json_object_set_new(j_issue, "auc_id", json_integer((int64_t)thisdb->Column(2)));
			json_object_set_new(j_issue, "mod_id", json_integer((int64_t)thisdb->Column(3)));

			j_crcs = json_array();

			for (int i=0; i<4; i++) {
				CRC[i] = (uint32_t)thisdb->Column(4+i);
				json_array_append_new(j_crcs, json_integer(CRC[i]));
			}

			j_echus = json_array();

			for (int i=0; i<4; i++) {
				ECHU[i] = (uint32_t)thisdb->Column(8+i);
				json_array_append_new(j_echus, json_integer(ECHU[i]));
			}

			double wubuf = thisdb->Column(13);
			double dhbuf = thisdb->Column(14);
			UniversalType verbuf = thisdb->Column(12);

			uint64_t exterrs = DataProcessing::AvalonError::DetectExtErrs((char *)verbuf.StringStore->c_str(), wubuf, dhbuf,
										      CRC[0]+CRC[1]+CRC[2]+CRC[3]);

			exterrs |= ECHU[0]|ECHU[1]|ECHU[2]|ECHU[3];

			json_object_set_new(j_issue, "crc", j_crcs);
			json_object_set_new(j_issue, "echu" , j_echus);
			json_object_set_new(j_issue, "echu_ored" , json_integer((int)exterrs));




			string sdna = strbindna(((pair<void *, size_t>)thisdb->Column(15)).first);

			json_object_set_new(j_issue, "dna", json_string(sdna.c_str()));

			string sebuf = DataProcessing::AvalonError::ToString(exterrs);

			json_object_set_new(j_issue, "msg" , json_string(sebuf.c_str()));

			pthread_mutex_lock(&thisCtx->Lock);
			json_array_append_new(thisCtx->j_issues, j_issue);
			pthread_mutex_unlock(&thisCtx->Lock);

		}

		delete thisdb;

	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

static void *getOtherErrors(void *userp){
	pCtx_i *thisCtx = (pCtx_i *)userp;

	json_t *j_issue, *j_crcs, *j_echus;

	uint32_t CRC[4];
	uint32_t ECHU[4];

	try {
		SQLAutomator::SQLite3 *thisdb = db_module_avalon7.OpenSQLite3();

		thisdb->Prepare("SELECT Addr, Port, Type FROM issue WHERE Time = ?1 AND Type >= 0x10 AND Type < 0x20");

		thisdb->Bind(1, thisCtx->LastDataCollection);


		while (thisdb->Step() == SQLITE_ROW) {
			j_issue = json_object();

			uint64_t typebuf = thisdb->Column(2);
			json_object_set_new(j_issue, "type", json_integer(typebuf));

			Reimu::IPEndPoint remoteEP((pair<void *, size_t>)thisdb->Column(0), thisdb->Column(1));

			json_object_set_new(j_issue, "ip", json_string(remoteEP.ToString(IPEndPoint::String_IP).c_str()));
			json_object_set_new(j_issue, "port", json_integer(remoteEP.Port));


			string sebuf = DataProcessing::Issue::ToString((uint64_t)typebuf);

			json_object_set_new(j_issue, "msg" , json_string(sebuf.c_str()));

			pthread_mutex_lock(&thisCtx->Lock);
			json_array_append_new(thisCtx->j_issues, j_issue);
			pthread_mutex_unlock(&thisCtx->Lock);

		}

		delete thisdb;

	} catch (Reimu::Exception e) {

	}

	pthread_exit(NULL);
}

int AMSD::Operations::issues(json_t *in_data, json_t *&out_data){

	pCtx_i thisCtx;

	thisCtx.j_issues = json_array();
	thisCtx.LastDataCollection = RuntimeData::TimeStamp::LastDataCollection();

	pthread_t p_gae, p_goe;

	pthread_create(&p_gae, NULL, &getAvalonErrors, &thisCtx);
	pthread_create(&p_goe, NULL, &getOtherErrors, &thisCtx);

	pthread_join(p_gae, NULL);
	pthread_join(p_goe, NULL);


	json_object_set_new(out_data, "issues", thisCtx.j_issues);

	return 0;
}