//
// Created by root on 17-4-19.
//

#ifndef AMSD_SERVICES_HPP
#define AMSD_SERVICES_HPP

#include "../amsd.hpp"

namespace AMSD {
    class Services {
    public:
	static map<string, Reimu::Tasker> Services;

	static int Init();

	static void API_UnixSocket(void *userp);
	static void API_TCP(void *userp);
	static void DataCollector(void *userp);
	static void MailReport(void *userp);
    };
}

#endif //AMSD_SERVICES_HPP
