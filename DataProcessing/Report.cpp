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
	SQLite3 thispooldb = db_pool.OpenSQLite3();

	uint8_t *blobuf;

	string sbuf0, sbuf1;
	char *cbuf0, *cbuf1;
	size_t lubuf0;

	Controller ctlbuf;
	Pool *poolbuf;


	thissummarydb.Prepare("SELECT Addr, Port, Elapsed, MHSav FROM summary WHERE Time = ?1 GROUP BY Addr, Port");

	thissummarydb.Bind(1, RuntimeData::TimeStamp::LastDataCollection());

	while (thissummarydb.Step() == SQLITE_ROW){

		blobuf = (uint8_t *)sqlite3_column_blob(stmtbuf0, 0);

		ctlbuf = Controller();
		ctlbuf.Addr.insert(ctlbuf.Addr.end(), blobuf, blobuf + sqlite3_column_bytes(stmtbuf0, 0));
		ctlbuf.Port = (uint16_t)sqlite3_column_int(stmtbuf0, 1);
		ctlbuf.Elapsed = (size_t)sqlite3_column_int(stmtbuf0, 2);

		Farm0.Controllers.push_back(ctlbuf);

		Farm0.MHS += sqlite3_column_double(stmtbuf0, 3);
	}


}

string DataProcessing::Report::HTMLReport() {
	return std::string();
}

