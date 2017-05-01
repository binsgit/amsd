//
// Created by root on 17-4-3.
//

#include "DataProcessing.hpp"


void DataProcessing::Collector::event_cb(struct bufferevent *bev, short events, void *ptr) {
	ConnectionContext *con_ctx = (ConnectionContext *)ptr;
	CgMinerAPI *apiProcessor = con_ctx->thisAPIProcessor;

	if (events & BEV_EVENT_CONNECTED) {
		fprintf(stderr,"amsd: Collector: %s (%p) connected\n", apiProcessor->RemoteEP->ToString().c_str(), bev);

		if (!apiProcessor->UserData) {
			string cmdstr = apiProcessor->QueryString();
			bufferevent_write(bev, cmdstr.c_str(), cmdstr.length());
			bufferevent_write(bev, "\n", 1);
			apiProcessor->UserData = 1;
		}

	} else {
		if (events & BEV_EVENT_EOF) {
			fprintf(stderr, "amsd: Collector: %s (%p) connection done (%d), %zu bytes received\n",
				apiProcessor->RemoteEP->ToString().c_str(), bev, events, apiProcessor->RawAPIData.size());
			apiProcessor->RawAPIData.push_back(0);
			try {
				apiProcessor->Process();
			} catch (Reimu::Exception e) {
				DataProcessing::Issue thisIssue(con_ctx->thisCollector->TimeStamp,
								DataProcessing::Issue::RawDataError,
								*apiProcessor->RemoteEP);
				thisIssue.WriteDatabase(apiProcessor->IssueDB);
			}
		} else {

			if (events & BEV_EVENT_ERROR) {
				if (apiProcessor->Type == CgMinerAPI::Summary) {
					DataProcessing::Issue thisIssue(con_ctx->thisCollector->TimeStamp,
									DataProcessing::Issue::ConnectionFailure,
									*apiProcessor->RemoteEP);
					thisIssue.WriteDatabase(apiProcessor->IssueDB);
				}
				fprintf(stderr, "amsd: datacollector: %s (%p) connection error (%d), %zu bytes received\n",
					apiProcessor->RemoteEP->ToString().c_str(), bev, events, apiProcessor->RawAPIData.size());
			} else if (events & BEV_EVENT_TIMEOUT) {
				if (apiProcessor->Type == CgMinerAPI::Summary) {
					DataProcessing::Issue thisIssue(con_ctx->thisCollector->TimeStamp,
									DataProcessing::Issue::ConnectionTimeout,
									*apiProcessor->RemoteEP);
					thisIssue.WriteDatabase(apiProcessor->IssueDB);
				}
				fprintf(stderr, "amsd: datacollector: %s (%p) connection timeout [%zu.%zu secs] (%d), %zu bytes received\n",
					apiProcessor->RemoteEP->ToString().c_str(), bev, con_ctx->thisCollector->TimeOut.tv_sec,
					con_ctx->thisCollector->TimeOut.tv_usec, events, apiProcessor->RawAPIData.size());
			}

		}

		bufferevent_free(bev);
		delete apiProcessor;
		delete con_ctx;
	}

}

void DataProcessing::Collector::conn_readcb(struct bufferevent *bev, void *user_data) {
	ConnectionContext *con_ctx = (ConnectionContext *)user_data;
	CgMinerAPI *apiProcessor = con_ctx->thisAPIProcessor;

	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);
	size_t n;

	char buf[4096];

	if (input_len > 0) {
		while (1) {
			n = bufferevent_read(bev, buf, sizeof(buf));
			if (n == 0)
				break;
			apiProcessor->RawAPIData.insert(apiProcessor->RawAPIData.end(), buf, buf + n);
		}

	}
}

void DataProcessing::Collector::conn_writecb(struct bufferevent *bev, void *user_data) {

}

void DataProcessing::Collector::Collect() {

	cerr << "amsd: DataProcessing::Collector: Collector started.\n";

	TimeStamp = time(NULL);
	RuntimeData::TimeStamp::CurrentDataCollection(TimeStamp);

	int rc;

	ConnectionContext *conCtx;

	CgMinerAPI *apiProcessor;

	void *remote_inaddr;
	uint16_t remote_port;
	int remote_addrlen;

	struct event_base *eventbase = NULL;
	struct bufferevent *bebuf = NULL;

	DBC_Controllers = db_controller.OpenSQLite3();
	DBC_Issue = db_issue.OpenSQLite3();

	fprintf(stderr, "[Collector @ %p] Connecting databases...\n", this);

	DBConnections.push_back(db_summary.OpenSQLite3());
	DBConnections.push_back(db_module_avalon7.OpenSQLite3());
	DBConnections.push_back(db_device.OpenSQLite3());
	DBConnections.push_back(db_pool.OpenSQLite3());

	for (auto &thisDBC : DBConnections) {
		thisDBC->Exec("BEGIN");
	}

	DBC_Controllers->Prepare(db_controller.Statement(SQLAutomator::SELECT_FROM));

	size_t sb_libev_workaround = 0;

	eventbase = event_base_new();

	vector<IPEndPoint *> Dustbin;

	while ( (rc = DBC_Controllers->Step()) == SQLITE_ROW ) {

		if (sb_libev_workaround > 192) {
			event_base_dispatch(eventbase);
			event_base_free(eventbase);
			eventbase = event_base_new();
			sb_libev_workaround = 0;
		}

		remote_inaddr = (void *)sqlite3_column_blob(DBC_Controllers->SQLite3Statement, 1);
		remote_addrlen = sqlite3_column_bytes(DBC_Controllers->SQLite3Statement, 1);
		remote_port = (uint16_t)sqlite3_column_int(DBC_Controllers->SQLite3Statement, 2);

		IPEndPoint *RemoteEP = new IPEndPoint(remote_inaddr, remote_addrlen, remote_port);
		Dustbin.push_back(RemoteEP);

		fprintf(stderr, "[Collector @ %p] Processing %s\n", this, RemoteEP->ToString().c_str());

		int apiType = 1;

		for (auto &thisDBC : DBConnections) {
			bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);
			bufferevent_set_timeouts(bebuf, &TimeOut, &TimeOut);

			if (bufferevent_socket_connect(bebuf, RemoteEP->SockAddr,
						       RemoteEP->AddressFamily == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6)) < 0) {
				bufferevent_free(bebuf);
				continue;
			} else {
				apiProcessor = new CgMinerAPI(TimeStamp, (CgMinerAPI::APIType)apiType, thisDBC, DBC_Issue, RemoteEP);
				conCtx = new ConnectionContext;
				conCtx->thisAPIProcessor = apiProcessor;
				conCtx->thisCollector = this;

				bufferevent_setcb(bebuf, conn_readcb, conn_writecb, event_cb, conCtx);
				bufferevent_enable(bebuf, EV_READ | EV_WRITE);
			}

			apiType++;
		}

		sb_libev_workaround++;

	}

	event_base_dispatch(eventbase);
	event_base_free(eventbase);

	fprintf(stderr, "[Collector @ %p] Disconnecting databases...\n", this);

	delete DBC_Issue;
	delete DBC_Controllers;

	for (auto &thisDBC : DBConnections) {
		thisDBC->Exec("COMMIT");
		delete thisDBC;
	}

	for (auto &thisGarbage : Dustbin)
		delete thisGarbage;

	RuntimeData::TimeStamp::LastDataCollection(RuntimeData::TimeStamp::CurrentDataCollection());



}


