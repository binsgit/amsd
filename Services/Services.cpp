//
// Created by root on 17-4-19.
//

#include "Services.hpp"

int Services::Init() {
	Services["API_UnixSocket"] = Tasker("API_UnixSocket", 1, &AMSD::Services::API::UnixSocket, NULL);
	Services["DataCollector"] = Tasker("DataCollector", 600, &AMSD::Services::DataCollector, NULL);
	Services["MailReport"] = Tasker("MailReport", 0, 8, -1, -1, -1, &AMSD::Services::MailReport, NULL);

	return 0;
}


