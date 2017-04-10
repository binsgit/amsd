//
// Created by root on 17-4-5.
//

#include "DataProcessing.hpp"

DataProcessing::Report::Report(string farm_name, bool collect_pool) {
	Name = farm_name;
	CollectPool = collect_pool;
	timeval tv_begin, tv_end;
	gettimeofday(&tv_begin, NULL);
	CollectData();
	gettimeofday(&tv_end, NULL);
	timersub(&tv_end, &tv_begin, &ProcessTime);
}

void DataProcessing::Report::CollectData() {
	SQLite3 thissummarydb = db_summary.OpenSQLite3();
	SQLite3 thismoduledb = db_module_avalon7.OpenSQLite3();


	uint8_t *blobuf;

	string sbuf0, sbuf1;
	char *cbuf0, *cbuf1;
	size_t lubuf0;


	Pool *poolbuf;



	thissummarydb.Prepare("SELECT Addr, Port, Elapsed, MHSav FROM summary WHERE Time = ?1 GROUP BY Addr, Port");

	thissummarydb.Bind(1, RuntimeData::TimeStamp::LastDataCollection());

	while (thissummarydb.Step() == SQLITE_ROW){
		Controller ctlbuf;

		ctlbuf.RemoteEP = IPEndPoint(thissummarydb.Column(0), thissummarydb.ColumnBytes(0), thissummarydb.Column(1));
		ctlbuf.Elapsed = thissummarydb.Column(2);
		Farm0.Controllers.push_back(ctlbuf);
		Farm0.MHS += thissummarydb.Column(3);
	}

	if (CollectPool) {
		SQLite3 thispooldb = db_pool.OpenSQLite3();

		thispooldb.Prepare("SELECT URL, User, DifficultyAccepted FROM pool WHERE "
					   "LENGTH(URL) > 7 AND "
					   "Time > ?1 AND "
					   "Addr = ?2 AND "
					   "Port = ?3 "
					   "GROUP BY URL");

		for (auto const &ctl: Farm0.Controllers) {
			thispooldb.Bind(1, RuntimeData::TimeStamp::LastDataCollection()-86400);
			thispooldb.Bind(2, {ctl.RemoteEP.Addr, ctl.RemoteEP.AddressFamily == AF_INET ? 4 : 16});
			thispooldb.Bind(3, ctl.RemoteEP.Port);

			while (thispooldb.Step() == SQLITE_ROW) {
				sbuf0 = thispooldb.Column(0);
				sbuf1 = thispooldb.Column(1);
				lubuf0 = thispooldb.Column(2);


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

			thispooldb.Reset();
		}

	}

	thismoduledb.Prepare("SELECT Count(*) FROM module_avalon7 WHERE Time = ?1");
	thismoduledb.Bind(1, RuntimeData::TimeStamp::LastDataCollection());

	thismoduledb.Step();

	Farm0.Modules = thismoduledb.Column(0);
}

string DataProcessing::Report::HTML() {
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

