//
// Created by root on 17-8-3.
//
#include "Operations.hpp"

#define CTLSCAN_STATUS_IDLE			0
#define CTLSCAN_STATUS_SCANNING			1
#define CTLSCAN_STATUS_CANCELING		2

static shared_timed_mutex GlobalLock;


static int Status = 0;
static size_t nr_ctls = 0, nr_procd_ctls = 0;

#define BE_ADD(x)				x = htobe32(be32toh(x)+1)

int AMSD::Operations::ctl_scanner(json_t *in_data, json_t *&out_data) {




	json_t *j_op = json_object_get(in_data, "op");

	string op;
	size_t index = 0;
	json_t *j_ip_st, *j_ip_ed;
	json_t *j_errmsg;
	sqlite3_stmt *stmt;

	const char *ip_st, *ip_ed;

	int rc;

	const void *addr;
	int addr_len;
	char addrsbuf[INET6_ADDRSTRLEN];
	time_t timenow;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);


	if (op == "status") {
		rc = Status;
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

		sockaddr_in ipst = {0};
		sockaddr_in iped = {0};

		inet_pton(AF_INET, ip_st, &ipst.sin_addr);
		inet_pton(AF_INET, ip_ed, &iped.sin_addr);

		ipst.sin_port = htobe16(4028);
		iped.sin_port = htobe16(4028);

		BE_ADD(ipst.sin_addr.s_addr);







	}


	return rc;

}