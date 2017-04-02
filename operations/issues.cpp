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


int AMSD::Operations::issues(json_t *in_data, json_t *&out_data){
	sqlite3 *thismoduledb, *thisissuedb;
	sqlite3_stmt *stmt;
	json_t *j_issues, *j_issue, *j_avalonerr;
	json_t *j_crcs, *j_echus;

	uint32_t CRC[4];
	uint32_t ECHU[4];

	size_t addrlen;
	char addrsbuf[64] = {0};
	u_char *addr;
	uint16_t port;
	string sebuf;
	int typebuf;

	u_char *verbuf;
	double wubuf, dhbuf;
	string sdna;

	j_issues = json_array();

//	Lock_DataCollector.lock();

	db_open(db_module_avalon7.DatabaseURI.c_str(), thismoduledb);

	sqlite3_prepare(thismoduledb, "SELECT Addr, Port, DeviceID, ModuleID, CRC_0, CRC_1, CRC_2, CRC_3, "
		"ECHU_0, ECHU_1, ECHU_2, ECHU_3, Ver, WU, DH, DNA "
		"FROM module_avalon7 WHERE Time = ?1 AND "
		"(ECHU_0 > 0 OR ECHU_1 > 0 OR ECHU_2 > 0 OR ECHU_3 > 0)", -1, &stmt, NULL);

	sqlite3_bind_int64(stmt, 1, RuntimeData::TimeStamp::LastDataCollection());

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		j_issue = json_object();
		json_object_set_new(j_issue, "type", json_integer(Issue::Issue::IssueType::AvalonError));

		addrlen = (size_t)sqlite3_column_bytes(stmt, 0);
		addr = (u_char *)sqlite3_column_blob(stmt, 0);
		port = (uint16_t)sqlite3_column_int(stmt, 1);

		inet_ntop(addrlen == 4 ? AF_INET : AF_INET6, addr, addrsbuf, addrlen == 4 ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
//		snprintf(addrsbuf+strlen(addrsbuf), 6, ":%" PRIu16, port);

		json_object_set_new(j_issue, "ip", json_string(addrsbuf));
		json_object_set_new(j_issue, "port", json_integer(port));
		json_object_set_new(j_issue, "auc_id", json_integer(sqlite3_column_int(stmt, 2)));
		json_object_set_new(j_issue, "mod_id", json_integer(sqlite3_column_int(stmt, 3)));

		j_crcs = json_array();

		for (int i=0; i<4; i++) {
			CRC[i] = (uint32_t)sqlite3_column_int(stmt, 4+i);
			json_array_append_new(j_crcs, json_integer(CRC[i]));
		}

		j_echus = json_array();

		for (int i=0; i<4; i++) {
			ECHU[i] = (uint32_t)sqlite3_column_int(stmt, 8+i);
			json_array_append_new(j_echus, json_integer(ECHU[i]));
		}

		json_object_set_new(j_issue, "crc", j_crcs);
		json_object_set_new(j_issue, "echu" , j_echus);

		verbuf = (u_char *)sqlite3_column_text(stmt, 12);
		wubuf = sqlite3_column_double(stmt, 13);
		dhbuf = sqlite3_column_double(stmt, 14);
		sdna = strbindna((void *)sqlite3_column_blob(stmt, 15));

		json_object_set_new(j_issue, "dna", json_string(sdna.c_str()));

		sebuf = Issue::Avalon_Error::strerror(ECHU[0]|ECHU[1]|ECHU[2]|ECHU[3]|
						      Issue::Avalon_Error::DetectExtErrs((char *)verbuf, wubuf, dhbuf,
											 CRC[0]+CRC[1]+CRC[2]+CRC[3]));

		json_object_set_new(j_issue, "msg" , json_string(sebuf.c_str()));

		json_array_append_new(j_issues, j_issue);

	}

	sqlite3_finalize(stmt);
	db_close(thismoduledb);


	db_open(db_issue.DatabaseURI.c_str(), thisissuedb);

	sqlite3_prepare(thisissuedb, "SELECT Addr, Port, Type FROM issue WHERE Time = ?1 "
		"AND Type >= 0x10 AND Type < 0x20", -1, &stmt, NULL);

	sqlite3_bind_int64(stmt, 1, RuntimeData::TimeStamp::LastDataCollection());

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		j_issue = json_object();

		typebuf = sqlite3_column_int(stmt, 2);
		json_object_set_new(j_issue, "type", json_integer(typebuf));

		addrlen = (size_t)sqlite3_column_bytes(stmt, 0);
		addr = (u_char *)sqlite3_column_blob(stmt, 0);
		port = (uint16_t)sqlite3_column_int(stmt, 1);

		inet_ntop(addrlen == 4 ? AF_INET : AF_INET6, addr, addrsbuf, addrlen == 4 ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);

		json_object_set_new(j_issue, "ip", json_string(addrsbuf));
		json_object_set_new(j_issue, "port", json_integer(port));


		sebuf = Issue::Issue::strerror((uint64_t)typebuf);

		json_object_set_new(j_issue, "msg" , json_string(sebuf.c_str()));

		json_array_append_new(j_issues, j_issue);

	}

	sqlite3_finalize(stmt);
	db_close(thisissuedb);

//	Lock_DataCollector.unlock();

	json_object_set_new(out_data, "issues", j_issues);

	return 0;
}