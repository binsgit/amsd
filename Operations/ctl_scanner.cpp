//
// Created by root on 17-8-3.
//
#include "Operations.hpp"

#define CTLSCAN_STATUS_IDLE			0
#define CTLSCAN_STATUS_SCANNING			1
#define CTLSCAN_STATUS_CANCELING		2

static shared_timed_mutex GlobalLock;


static size_t nr_ctls = 0, nr_procd_ctls = 0;

#define BE_ADD(x)				x = htobe32(be32toh(x)+1)

static sockaddr_in ipst;
static sockaddr_in iped;
static sockaddr_in curnode;

static size_t ccnt, cuccnt;

static uint32_t lastfound;

static vector<uint32_t> foundlist;
static int scan_rc = -1;
static int scan_status = 0;

static void event_cb(struct bufferevent *bev, short events, void *ptr){
	if (events & BEV_EVENT_CONNECTED || events & BEV_EVENT_EOF) {
		foundlist.push_back((uint32_t)ptr);
		lastfound = (uint32_t)ptr;
	} else {

	}

	cuccnt++;
	bufferevent_free(bev);
}

static void wcb(struct bufferevent *bev, void *user_data){

}

static void rcb(struct bufferevent *bev, void *user_data){

}

static void *scanner(void *userp) {
	cuccnt = 0;
	ccnt = be32toh(iped.sin_addr.s_addr) - be32toh(ipst.sin_addr.s_addr);

	if (ccnt < 1) {
		scan_rc = 1;
		scan_status = 0;
		GlobalLock.unlock();
		pthread_exit(NULL);
	}


	struct event_base *eventbase = event_base_new();
	struct bufferevent *bebuf = NULL;

	if (!eventbase){
		scan_rc = 2;
		scan_status = 0;
		GlobalLock.unlock();
		pthread_exit(NULL);
	}



	memcpy(&curnode, &ipst, sizeof(sockaddr_in));

	struct timeval TimeOut = {15, 0};

	for (size_t i=0; i<ccnt; i++) {
		bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_set_timeouts(bebuf, &TimeOut, &TimeOut);

		if (bufferevent_socket_connect(bebuf, (struct sockaddr *)&curnode, sizeof(sockaddr_in))) {
			bufferevent_free(bebuf);
			cuccnt++;
		} else {
			bufferevent_setcb(bebuf, rcb, wcb, event_cb, (void *)curnode.sin_addr.s_addr);
			bufferevent_enable(bebuf, EV_READ | EV_WRITE);
		}

		BE_ADD(curnode.sin_addr.s_addr);
	}

	event_base_dispatch(eventbase);
	event_base_free(eventbase);

	scan_rc = 0;
	scan_status = 0;
	GlobalLock.unlock();
	pthread_exit(NULL);

}

int AMSD::Operations::ctl_scanner(json_t *in_data, json_t *&out_data) {


	json_t *j_op = json_object_get(in_data, "op");

	string op;
	size_t index = 0;
	json_t *j_ip_st, *j_ip_ed;
	json_t *j_errmsg;
	sqlite3_stmt *stmt;

	const char *ip_st, *ip_ed;

	int rc = 0;

	const void *addr;
	int addr_len;
	char addrsbuf[INET6_ADDRSTRLEN];
	time_t timenow;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);


	if (op == "status") {
		json_object_set_new(out_data, "scan_status", json_integer(scan_status));
		json_object_set_new(out_data, "scan_rc", json_integer(scan_rc));

		in_addr atmp;
		atmp.s_addr = lastfound;
		char asbuf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET, &atmp, asbuf, INET_ADDRSTRLEN);
		json_object_set_new(out_data, "lastfound", json_string(asbuf));


	} else if (op == "scan") {
		auto lock_status = GlobalLock.try_lock();

		if (!lock_status) {
			return 666;
		}

		j_ip_st = json_object_get(in_data, "ip_st");
		j_ip_ed = json_object_get(in_data, "ip_ed");

		if (!json_is_string(j_ip_ed) || !json_is_string(j_ip_st))
			return -1;


		ip_st = json_string_value(j_ip_st);
		ip_ed = json_string_value(j_ip_ed);

		memset(&ipst, 0, sizeof(sockaddr_in));
		memset(&iped, 0, sizeof(sockaddr_in));


		inet_pton(AF_INET, ip_st, &ipst.sin_addr);
		inet_pton(AF_INET, ip_ed, &iped.sin_addr);

		ipst.sin_port = htobe16(4028);
		iped.sin_port = htobe16(4028);



		pthread_t tid;

		scan_status = 1;
		pthread_create(&tid, &_pthread_detached, &scanner, NULL);

	} else if (op == "result") {
		json_t *j_result_arr = json_array();

		for (auto &tr: foundlist) {

		}
	}


	return rc;

}