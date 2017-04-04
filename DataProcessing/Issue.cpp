//
// Created by root on 17-4-3.
//

#include "DataProcessing.hpp"



DataProcessing::Issue::Issue() {

}

DataProcessing::Issue::Issue(time_t time_now, DataProcessing::Issue::IssueType type, Reimu::IPEndPoint remoteEP) {
	Time = time_now;
	Type = type;
	RemoteEP = remoteEP;
}

DataProcessing::Issue::Issue(time_t time_now, Reimu::IPEndPoint remoteEP, uint64_t errnum, uint16_t AUC, uint16_t Module, uint64_t DNA) {
	Time = time_now;
	Type = AvalonError;
	RemoteEP = remoteEP;

	Error_Avalon = new AvalonError(errnum, AUC, Module, DNA);
}

DataProcessing::Issue::~Issue() {
	if (Error_Avalon)
		delete Error_Avalon;
}

void DataProcessing::Issue::WriteDatabase(Reimu::SQLAutomator::SQLite3 *db) {

	db->Prepare(db_issue.Statement(Reimu::SQLAutomator::INSERT_INTO));

	db->Bind(1, Time);
	db->Bind(2, {RemoteEP.Addr, RemoteEP.AddressFamily == AF_INET ? 4 : 16});
	db->Bind(3, RemoteEP.Port);
	db->Bind(4, Type);

	if (Type == AvalonError) {
		vector<uint8_t> descbuf = Error_Avalon->Desc();
		db->Bind(5, descbuf);
	}

	db->Step();
}

