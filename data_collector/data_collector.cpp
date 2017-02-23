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
	CgMiner_APIBuf *apibuf = (CgMiner_APIBuf *)ptr;

	if (events & BEV_EVENT_CONNECTED) {
		fprintf(stderr,"amsd: datacollector: buffer event %p connected\n", bev);
		const string *cmdstr = &api_cmd_summary;

		if (!apibuf->CmdWritten) {
			switch (apibuf->Type) {
				case CgMiner_APIBuf::Summary:
					break;
				case CgMiner_APIBuf::EStats:
					cmdstr = &api_cmd_estats;
					break;
				case CgMiner_APIBuf::EDevs:
					cmdstr = &api_cmd_edevs;
					break;
				case CgMiner_APIBuf::Pools:
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

	CgMiner_APIBuf *apibuf = (CgMiner_APIBuf *)user_data;
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);
	size_t n;

	char buf[4096];

	if (input_len > 0) {
		while (1) {
			n = bufferevent_read(bev, buf, sizeof(buf));
			fprintf(stderr,"amsd: datacollector: buffer event %p read %zu bytes\n", bev, n);
			if (n <= 0) {
				apibuf->Buf.push_back(0);
				break;
			}
			apibuf->Buf.insert(apibuf->Buf.begin(), buf, buf + n);
		}

	}

}

static void conn_writecb(struct bufferevent *bev, void *user_data){
	fprintf(stderr,"amsd: datacollector: buffer event %p ready for writing\n", bev);


}

void amsd_datacollector_instance(){
	struct event_base *eventbase = event_base_new();
	struct bufferevent *bebuf = NULL;
	struct sockaddr_in remote_addr;
	time_t timenow;
	CgMiner_APIBuf *abuf;

	bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);

	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(4028);
	remote_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	timenow = time(NULL);



	if (bufferevent_socket_connect(bebuf, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr_in)) < 0) {
		bufferevent_free(bebuf);
	} else {

		abuf = new CgMiner_APIBuf(CgMiner_APIBuf::Summary, timenow, remote_addr.sin_addr.s_addr, 4028);
		bufferevent_setcb(bebuf, conn_readcb, conn_writecb, event_cb, abuf);
		bufferevent_enable(bebuf, EV_READ | EV_WRITE);
	}

	event_base_dispatch(eventbase);
	event_base_free(eventbase);
}