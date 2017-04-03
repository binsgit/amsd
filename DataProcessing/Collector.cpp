//
// Created by root on 17-4-3.
//

#include "DataProcessing.hpp"


void DataProcessing::Collector::event_cb(struct bufferevent *bev, short events, void *ptr) {
	CgMinerAPI *apiProcessor = (CgMinerAPI *)ptr;

	if (events & BEV_EVENT_CONNECTED) {
		fprintf(stderr,"amsd: Collector: %s (%p) connected\n", apiProcessor->RemoteEP.ToString(), bev);

		if (!apiProcessor->UserData) {
			string cmdstr = apiProcessor->QueryString();
			bufferevent_write(bev, cmdstr.c_str(), cmdstr.length());
			bufferevent_write(bev, "\n", 1);
			apiProcessor->UserData = 1;
		}

	} else {
		if (events & BEV_EVENT_EOF) {
			fprintf(stderr, "amsd: Collector: %s (%p) connection done (%d), %zu bytes received\n",
				apiProcessor->RemoteEP.ToString().c_str(), bev, events, apiProcessor->RawAPIData.size());
			apiProcessor->Process();
		} else {
			sqlite3_bind_int64(stmt_log_timeout, 1, apibuf->StartTime);
			sqlite3_bind_blob64(stmt_log_timeout, 2, apibuf->Remote_Addr, apibuf->Remote_AddrLen, SQLITE_STATIC);
			sqlite3_bind_int(stmt_log_timeout, 3, apibuf->Remote_Port);

			if (events & BEV_EVENT_ERROR) {
				sqlite3_bind_int(stmt_log_timeout, 4, Issue::Issue::IssueType::ConnectionFailure);
				fprintf(stderr, "amsd: datacollector: %s (%p) connection error (%d), %zu bytes received\n",
					apibuf->Remote_AddrText.c_str(), bev, events, apibuf->NetIOBuf.size());
			} else if (events & BEV_EVENT_TIMEOUT) {
				sqlite3_bind_int(stmt_log_timeout, 4, Issue::Issue::IssueType::ConnectionTimeout);
				fprintf(stderr, "amsd: datacollector: %s (%p) connection timeout [%zu.%zu secs] (%d), %zu bytes received\n",
					apibuf->Remote_AddrText.c_str(), bev, amsd_datacollection_conntimeout.tv_sec,
					amsd_datacollection_conntimeout.tv_usec, events, apibuf->NetIOBuf.size());
			}

			sqlite3_step(stmt_log_timeout);
			sqlite3_reset(stmt_log_timeout);
		}

		bufferevent_free(bev);
		delete apibuf;
	}

}

void DataProcessing::Collector::conn_readcb(struct bufferevent *bev, void *user_data) {

}

void DataProcessing::Collector::conn_writecb(struct bufferevent *bev, void *user_data) {

}

