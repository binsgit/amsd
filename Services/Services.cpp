//
// Created by root on 17-4-19.
//

#include "Services.hpp"

map<string, Reimu::Tasker> AMSD::Services::ServicesList;

int AMSD::Services::Init() {
	ServicesList["API_UnixSocket"] = Tasker("API_UnixSocket", 1, &AMSD::Services::API::UnixSocket, NULL);
	ServicesList["DataCollector"] = Tasker("DataCollector", 600, &AMSD::Services::DataCollector, NULL);
	ServicesList["MailReport"] = Tasker("MailReport", 0, 8, -1, -1, -1, &AMSD::Services::MailReport, NULL, 0);

	for (auto &thisSvc : ServicesList) {
		cerr << "amsd: Services::Init: Starting service " << thisSvc.first << endl;
		thisSvc.second.Start();
	}

	return 0;
}


