//
// Created by root on 17-2-15.
//

#include "../amsd.hpp"

static const string api_cmd_summary = "{\"command\":\"summary\"}";
static const string api_cmd_estats = "{\"command\":\"estats\"}";
static const string api_cmd_edevs = "{\"command\":\"edevs\"}";
static const string api_cmd_pools = "{\"command\":\"pools\"}";

map<ReimuInetAddr, Avalon_Controller> Controllers;

void *amsd_datacollector_thread(void *meow){

}

static void event_cb(struct bufferevent *bev, short events, void *ptr){
	CgMinerAPIProcessor *apibuf = (CgMinerAPIProcessor *)ptr;

	if (events & BEV_EVENT_CONNECTED) {
		fprintf(stderr,"amsd: datacollector: buffer event %p connected\n", bev);
		const string *cmdstr = &api_cmd_summary;

		if (!apibuf->CmdWritten) {
			switch (apibuf->Type) {
				case CgMinerAPIProcessor::Summary:
					break;
				case CgMinerAPIProcessor::EStats:
					cmdstr = &api_cmd_estats;
					break;
				case CgMinerAPIProcessor::EDevs:
					cmdstr = &api_cmd_edevs;
					break;
				case CgMinerAPIProcessor::Pools:
					cmdstr = &api_cmd_pools;
					break;
			}

			bufferevent_write(bev, cmdstr->c_str(), cmdstr->length());
			bufferevent_write(bev, "\n", 1);
			apibuf->CmdWritten = 1;
		}

	} else if (events & BEV_EVENT_EOF) {
		fprintf(stderr,"amsd: datacollector: buffer event %p done\n", bev);
		unsigned char *s = &apibuf->Buf[0];
		puts((const char *)s);
		apibuf->Buf.push_back(0);
		apibuf->Process();
		delete apibuf;
		bufferevent_free(bev);
	} else if (events & BEV_EVENT_ERROR) {
		fprintf(stderr,"amsd: datacollector: buffer event %p error\n", bev);
		delete apibuf;
		bufferevent_free(bev);
	}
}

static void conn_readcb(struct bufferevent *bev, void *user_data){
	fprintf(stderr,"amsd: datacollector: buffer event %p ready for reading\n", bev);

	CgMinerAPIProcessor *apibuf = (CgMinerAPIProcessor *)user_data;
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);
	size_t n;

	char buf[4096];

	if (input_len > 0) {
		while (1) {
			n = bufferevent_read(bev, buf, sizeof(buf));
			fprintf(stderr,"amsd: datacollector: buffer event %p read %zu bytes\n", bev, n);
			if (n == 0)
				break;
			apibuf->Buf.insert(apibuf->Buf.end(), buf, buf + n);
		}

	}

}

static void conn_writecb(struct bufferevent *bev, void *user_data){
	fprintf(stderr,"amsd: datacollector: buffer event %p ready for writing\n", bev);

}

void amsd_datacollector_instance(){
	struct event_base *eventbase = event_base_new();
	struct bufferevent *bebuf = NULL;
	uint8_t remote_sockaddr[sizeof(struct sockaddr_in6)];
	time_t timenow;
	sqlite3 *thisdb;
	sqlite3_stmt *stmt;
	CgMinerAPIProcessor *abuf;
	const void *remote_inaddr;
	uint16_t remote_port;
	int inaddr_len, sockaddr_len;
	char addrsbuf[INET6_ADDRSTRLEN];
	int rc;

	timenow = time(NULL);

	db_open(dbpath_controller, thisdb);

	sqlite3_prepare_v2(thisdb, "SELECT * FROM controller", -1, &stmt, NULL);

	while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW ) {

		remote_inaddr = sqlite3_column_blob(stmt, 1);
		inaddr_len = sqlite3_column_bytes(stmt, 1);
		remote_port = (uint16_t)sqlite3_column_int(stmt, 2);

		if (inaddr_len == 4) {
			((struct sockaddr_in *)remote_sockaddr)->sin_family = AF_INET;
			((struct sockaddr_in *)&remote_sockaddr)->sin_port = htons(remote_port);
			memcpy(&((struct sockaddr_in *)remote_sockaddr)->sin_addr, remote_inaddr, 4);
			sockaddr_len = sizeof(struct sockaddr_in);
		} else if (inaddr_len == 16) {
			((struct sockaddr_in6 *)remote_sockaddr)->sin6_family = AF_INET6;
			((struct sockaddr_in6 *)&remote_sockaddr)->sin6_port = htons(remote_port);
			memcpy(&((struct sockaddr_in6 *)remote_sockaddr)->sin6_addr, remote_inaddr, 16);
			sockaddr_len = sizeof(struct sockaddr_in6);
		} else {
			// TODO: Internal err handling
			continue;
		}

		for (int apicat = 1; apicat <= 4; apicat++) {
			bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);

			if (bufferevent_socket_connect(bebuf, (struct sockaddr *)remote_sockaddr, sockaddr_len) < 0) {
				bufferevent_free(bebuf);
				continue;
			} else {
				abuf = new CgMinerAPIProcessor((CgMinerAPIProcessor::CgMiner_APIType)apicat, timenow, remote_inaddr, (size_t)inaddr_len, remote_port);
				bufferevent_setcb(bebuf, conn_readcb, conn_writecb, event_cb, abuf);
				bufferevent_enable(bebuf, EV_READ | EV_WRITE);
			}
		}

	}

	sqlite3_finalize(stmt);

	db_close(thisdb);

	event_base_dispatch(eventbase);
	event_base_free(eventbase);
}