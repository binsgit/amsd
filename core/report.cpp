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

#include "report.hpp"


Report::Report::Report(string farm_name, bool collect_pool) {
	Name = farm_name;
	CollectPool = collect_pool;
	timeval tv_begin, tv_end;
	gettimeofday(&tv_begin, NULL);
	CollectData();
	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_begin, &ProcessTime);
}

void Report::Report::CollectData() {
	sqlite3 *thissummarydb, *thismoduledb, *thispooldb;
	sqlite3_stmt *stmtbuf0, *stmtbuf1, *stmtbuf2;


	uint8_t *blobuf;

	string sbuf0, sbuf1;
	char *cbuf0, *cbuf1;
	size_t lubuf0;

	Controller ctlbuf;
	Pool *poolbuf;

//	Lock_DataCollector.lock();

	db_open(dbpath_summary, thissummarydb);
	db_open(dbpath_module_avalon7, thismoduledb);



	sqlite3_prepare_v2(thissummarydb, "SELECT Addr, Port, Elapsed, MHSav FROM summary WHERE Time = ?1 GROUP BY Addr, Port", -1, &stmtbuf0, NULL);
	sqlite3_bind_int64(stmtbuf0, 1, RuntimeData::TimeStamp::LastDataCollection());

	while (sqlite3_step(stmtbuf0) == SQLITE_ROW){

		blobuf = (uint8_t *)sqlite3_column_blob(stmtbuf0, 0);

		ctlbuf = Controller();
		ctlbuf.Addr.insert(ctlbuf.Addr.end(), blobuf, blobuf + sqlite3_column_bytes(stmtbuf0, 0));
		ctlbuf.Port = (uint16_t)sqlite3_column_int(stmtbuf0, 1);
		ctlbuf.Elapsed = (size_t)sqlite3_column_int(stmtbuf0, 2);

		Farm0.Controllers.push_back(ctlbuf);

		Farm0.MHS += sqlite3_column_double(stmtbuf0, 3);
	}

	sqlite3_finalize(stmtbuf0);


	if (CollectPool) {
		db_open(dbpath_pool, thispooldb);
		sqlite3_prepare_v2(thispooldb, "SELECT URL, User, DifficultyAccepted FROM pool WHERE "
			"LENGTH(URL) > 7 AND "
			"Time > ?1 AND "
			"Addr = ?2 AND "
			"Port = ?3 "
			"GROUP BY URL", -1, &stmtbuf1, NULL);

		for (auto const &ctl: Farm0.Controllers) {

			sqlite3_bind_int64(stmtbuf1, 1, RuntimeData::TimeStamp::LastDataCollection()-86400);
			sqlite3_bind_blob64(stmtbuf1, 2, &ctl.Addr[0], ctl.Addr.size(), SQLITE_STATIC);
			sqlite3_bind_int(stmtbuf1, 3, ctl.Port);


			while (sqlite3_step(stmtbuf1) == SQLITE_ROW) {
				sbuf0 = string((char *) sqlite3_column_text(stmtbuf1, 0));
				sbuf1 = string((char *) sqlite3_column_text(stmtbuf1, 1));
				lubuf0 = (size_t) sqlite3_column_int64(stmtbuf1, 2);


				cbuf0 = (char *)strchr(sbuf1.c_str(), '.');

				if (cbuf0) {
					cbuf1 = strdupa(sbuf1.c_str());
					cbuf1[cbuf0-sbuf1.c_str()] = 0;
					sbuf1 = string(cbuf1);
				}


				poolbuf = &Farm0.Pools[pair<string, string>(sbuf0, sbuf1)];

				poolbuf->URL = sbuf0;
				poolbuf->User = sbuf1;
				poolbuf->GHS += diffaccept2ghs(lubuf0, ctl.Elapsed);

				cerr << diffaccept2ghs(lubuf0, ctl.Elapsed) << endl;
			}

			sqlite3_reset(stmtbuf1);

		}

		sqlite3_finalize(stmtbuf1);

		db_close(thispooldb);
	}

	sqlite3_prepare_v2(thismoduledb, "SELECT Count(*) FROM module_avalon7 WHERE Time = ?1", -1, &stmtbuf2, NULL);
	sqlite3_bind_int64(stmtbuf2, 1, RuntimeData::TimeStamp::LastDataCollection());
	sqlite3_step(stmtbuf2);

	Farm0.Modules = (size_t)sqlite3_column_int64(stmtbuf2, 0);

	sqlite3_finalize(stmtbuf2);

	db_close(thissummarydb);
	db_close(thismoduledb);


//	Lock_DataCollector.unlock();
}

string Report::Report::HTMLReport() {
	char sbuf16[16];

	string ret = "<html>"
			     "<head>"
			     "<meta charset=\"UTF-8\">"
			     "<style>"
			     "th {"
			     "    border: 1px solid black;"
			     "    text-align: left;"
			     "    font-family: Microsoft Yahei;"
			     "}"
			     "table {"
			     "    border-collapse: collapse;"
			     "    border: 1px solid black;"
			     "}"
			     "</style>"
			     "</head>"
			     "<body>"
			     "<h3><b>AMS报告：" + Name + "</b></h3>"
			     "<h4><b>当前概况</b></h4><table><tbody>"
			     "<tr><th>数据采集时间</th><th>" + rfc3339_strftime(RuntimeData::TimeStamp::LastDataCollection()) + "</th></tr>"
			     "<tr><th>总算力</th><th>" + hashrate_h(Farm0.MHS) + "</th></tr><tr><th>"
			     "控制器数量</th><th>" + to_string(Farm0.Controllers.size()) + "</th></tr>"
			     "<tr><th>模组数量</th><th>" + to_string(Farm0.Modules) + "</th></tr></tbody></table><h4><b>矿池信息</b></h4>"
			     "<table><thead>"
			     "<tr><th>URL</th><th>用户</th><th>算力</th>"
//					   "<th>关联的控制器数量</th><th>关联的模组数量</th>"
			     "</tr>"
			     "</thead><tbody>";


	for (auto const &pool: Farm0.Pools) {
		ret += "<tr><th>";
		ret += pool.second.URL;
		ret += "</th><th>";
		ret += pool.second.User;
		ret += "</th><th>";
		ret += hashrate_h(pool.second.GHS*1000);
//		ret += "</th><th>";
//
//		ret += "</th><th>";
		ret += "</th></tr>";
	}

	ret += "</tbody></table><br><i>Processed in ";
	snprintf(sbuf16, 15, "%zu.%zu", ProcessTime.tv_sec, ProcessTime.tv_usec);
	ret += sbuf16;
	ret += " seconds.</i></body></html>";

	return ret;
}


