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

	class API {
	    struct ConCtx {
		int fd;
		pthread_t tid;

		vector<uint8_t> *buf_in;
		vector<uint8_t> *buf_out;
	    };

	    static int ParseRequest(char *inputstr, string &outputstr);
	    static void ConnectionHandler(ConCtx *data);
	    static void *ConnectionThread(void *data);

	public:

	    static void UnixSocket(void *userp);
	    static void TCP(void *userp);

	};


	static void DataCollector(void *userp);
	static void MailReport(void *userp);
    };
}

#endif //AMSD_SERVICES_HPP
