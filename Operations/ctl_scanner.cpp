//
// Created by root on 17-8-3.
//
#include "Operations.hpp"

#define CTLSCAN_STATUS_IDLE			0
#define CTLSCAN_STATUS_SCANNING			1
#define CTLSCAN_STATUS_CANCELING		2

static shared_timed_mutex GlobalLock;

#define BE_ADD(x)				x = htobe32(be32toh(x)+1)

static struct sockaddr_in ipst;
static struct sockaddr_in iped;
static struct sockaddr_in curnode;

static size_t ccnt, cuccnt, valid_cnt;

static uint32_t lastip;

static vector<uint32_t> foundlist;
static int scan_rc = -1;
static int scan_status = 0;

static void event_cb(struct bufferevent *bev, short events, void *ptr){

	uint32_t addr = (uint32_t)(intptr_t)ptr;

	lastip = addr;

	if (events & BEV_EVENT_CONNECTED || events & BEV_EVENT_EOF) {
		LogD("Found IPv4 addr %08x", be32toh(addr));
		foundlist.push_back(addr);
		valid_cnt++;
//		lastip = addr;
	} else {
		LogD("Bad IPv4 addr %08x", be32toh(addr));
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
	valid_cnt = 0;
	foundlist.clear();
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


	memcpy(&curnode, &ipst, sizeof(struct sockaddr_in));
	curnode.sin_family = AF_INET;

	struct timeval TimeOut = {7, 0};

	size_t sb_libev_workaround = 0;


	for (size_t i=0; i<ccnt; i++) {

		if (sb_libev_workaround > 48) {
			event_base_dispatch(eventbase);
			event_base_free(eventbase);
			eventbase = event_base_new();
			sb_libev_workaround = 0;
		}

		LogD("Checking IPv4 addr %08x", be32toh(curnode.sin_addr.s_addr));

		bebuf = bufferevent_socket_new(eventbase, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_set_timeouts(bebuf, &TimeOut, &TimeOut);

		if (-1 == bufferevent_socket_connect(bebuf, (struct sockaddr *)&curnode, sizeof(struct sockaddr_in))) {
			bufferevent_free(bebuf);
			cuccnt++;
			LogD("IPv4 addr %08x connect error", be32toh(curnode.sin_addr.s_addr));
		} else {
			bufferevent_setcb(bebuf, rcb, wcb, event_cb, (void *)(intptr_t)curnode.sin_addr.s_addr);
			bufferevent_enable(bebuf, EV_READ | EV_WRITE);
		}

		BE_ADD(curnode.sin_addr.s_addr);
		sb_libev_workaround++;
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
		json_object_set_new(out_data, "ccnt", json_integer(ccnt));
		json_object_set_new(out_data, "cuccnt", json_integer(cuccnt));
		json_object_set_new(out_data, "valid_cnt", json_integer(valid_cnt));

		in_addr atmp;
		atmp.s_addr = lastip;
		char asbuf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET, &atmp, asbuf, INET_ADDRSTRLEN);
		json_object_set_new(out_data, "lastip", json_string(asbuf));


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

		if (be32toh(ipst.sin_addr.s_addr) >= be32toh(iped.sin_addr.s_addr))
			return 233;

		ipst.sin_port = htobe16(4028);
		iped.sin_port = htobe16(4028);


		pthread_t tid;

		scan_status = 1;
		pthread_create(&tid, &_pthread_detached, &scanner, NULL);

	} else if (op == "result") {
		json_t *j_result_arr = json_array();

		for (auto &tr: foundlist) {
			string ipstr = IPEndPoint::ToString(tr);

			json_array_append_new(j_result_arr, json_string(ipstr.c_str()));

		}

		json_object_set_new(out_data, "results", j_result_arr);
	}


	return rc;

}