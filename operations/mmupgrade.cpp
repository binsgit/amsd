/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../amsd.hpp"

static pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;
map<string, MMUpgrade *> upgrades;


void MMUpgrade::MMUpgradeInstance(MMUpgrade *m){
	m->DownloadPercent = 0;
	m->UpgradePercent = 0;
	m->Status = Uninitialized;
	m->Message(m, "");

	SSHConnection *DownloadSession = new SSHConnection();
	DownloadSession->IP = m->IP;
	DownloadSession->UserName = "root";
	DownloadSession->Command = "wget -O /tmp/mm.mcs " + m->URL_MCS + " 2>&1";
	DownloadSession->NonBlockingExec();

	vector<uint8_t> buf;
	char *bp, *bp2;

	m->Status = DownloadInProgress;

	while (DownloadSession->Status != Finished) {
		sleep(1);

		if (DownloadSession->ErrorMessage.str() != "") {
			break;
		}

		buf = DownloadSession->GetReadBuffer();

		if (DownloadSession->Status == ExecInProgress) {
			bp = strchr((char *) &(buf[0]), '%');

			if (bp) {
				m->DownloadPercent = (int) strtol(bp - 2, NULL, 10);
				Message(m, "正在下载固件：" + to_string(m->DownloadPercent) + "%");
			} else {
				Message(m, "正在连接固件下载服务器");
			}
		} else {
			Message(m, "正在连接控制器");
		}

		cerr << m->_Message << "\n";

	}

	if (DownloadSession->CommandExitCode == 0) {
		Message(m, "固件下载完成");
		m->Status = DownloadFinished;
		fprintf(stderr, "amsd: mmupgrade: IP %s: Finished downloading\n", m->IP.c_str());
		m->DownloadPercent = 100;
	} else {
		Message(m, "固件下载失败: "+DownloadSession->ErrorMessage.str());
		m->Status = DownloadError;
		fprintf(stderr, "amsd: mmupgrade: IP %s: Download failed\n", m->IP.c_str());
	}

	delete DownloadSession;

	if (m->Status != DownloadFinished)
		return;


	SSHConnection *UpgradeSession = new SSHConnection();
	UpgradeSession->IP = m->IP;
	UpgradeSession->UserName = "root";
	UpgradeSession->Command = "mmupgrade 2>&1";
	UpgradeSession->NonBlockingExec();

	m->Status = UpgradeInProgress;

	while (UpgradeSession->Status != Finished) {
		sleep(1);
		buf = UpgradeSession->GetReadBuffer();

		bp2 = (char *)&(buf[0]);

		if (strstr(bp2, "finished mboot")) {
			m->Status = WaitingFinish;
			Message(m, "升级成功，请等待重启");
			m->UpgradePercent = 96;
		} else {
			m->Status = UpgradeInProgress;
			if (strstr(bp2, "Program Success")) {
				Message(m, "MM Flash 烧写成功");
				m->UpgradePercent = 95;
			} else if (strstr(bp2, "Program Begin")) {
				Message(m, "正在烧写 MM Flash");
				m->UpgradePercent = 50;
			} else if (strstr(bp2, "Erase Success")) {
				Message(m, "MM Flash 擦除成功");
				m->UpgradePercent = 40;
			} else if (strstr(bp2, "Erase Begin")) {
				Message(m, "正在擦除 MM Flash");
				m->UpgradePercent = 10;
			} else if (strstr(bp2, "Enter mboot mode")) {
				Message(m, "正在进入 mboot 模式");
				m->UpgradePercent = 10;
			} else {
				Message(m, "正在准备升级");
				m->UpgradePercent = 0;
			}

		}

		cerr << m->_Message << "\n";

	}

	if (m->Status == WaitingFinish) {
		m->Status = Finished;
		m->UpgradePercent = 100;
		Message(m, "升级完成");
		fprintf(stderr, "amsd: mmupgrade: IP %s: Finished upgrading\n", m->IP.c_str());
	} else {
		m->Status = UpgradeError;
		Message(m, "升级失败");
		fprintf(stderr, "amsd: mmupgrade: IP %s: Upgrade failed\n", m->IP.c_str());
	}

	delete UpgradeSession;

}

void *MMUpgrade::MMUpgradeThread(void *p) {
	MMUpgradeInstance((MMUpgrade *)p);
	pthread_exit(NULL);
}

void MMUpgrade::Exec() {
	pthread_create(&ThreadId, &_pthread_detached, &MMUpgradeThread, this);
}

string MMUpgrade::Message() {
	string ret;
	pthread_mutex_lock(&mutex_msg);
	ret = _Message;
	pthread_mutex_unlock(&mutex_msg);
	return ret;
}

void MMUpgrade::Message(MMUpgrade *mu, string m) {
	pthread_mutex_lock(&mu->mutex_msg);
	mu->_Message = m;
	pthread_mutex_unlock(&mu->mutex_msg);
}

void amsd_mmupgrade_new(string ip, string url_mcs){
	MMUpgrade *tmu;

	pthread_mutex_lock(&map_lock);
	auto found = upgrades.find(ip);
	if (found == upgrades.end()) {
		tmu = new MMUpgrade();
		upgrades.insert(pair<string, MMUpgrade *>(ip, tmu));
	} else {
		tmu = found->second;
	}
	pthread_mutex_unlock(&map_lock);

	tmu->IP = ip;
	tmu->URL_MCS = url_mcs;
	tmu->Exec();
}

void amsd_mmupgrade_clear(){
	pthread_mutex_lock(&map_lock);
	for (auto thisupd = upgrades.begin(); thisupd != upgrades.end(); thisupd++) {
		MMUpgrade::MMUpdStatus status = thisupd->second->Status;
		if (status == 0||status == 0x31||status == 0x32||status == 0x51) {
			delete thisupd->second;
			upgrades.erase(thisupd->first);
		}
	}
	pthread_mutex_unlock(&map_lock);
}

int amsd_operation_mmupgrade(json_t *in_data, json_t *&out_data){
	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_st_sts, *j_st_obj_buf, *j_st_buf;
	json_t *j_up_ips, *j_up_thisip;
	json_t *j_fw_url;
	size_t i;
	long upds_cnt = 0;
	const char *csbuf;
	string op;

	if (!j_op || !json_is_string(j_op))
		return -1;

	op = json_string_value(j_op);

	if (op == "status") {
		j_st_sts = json_array();

		pthread_mutex_lock(&map_lock);
		for (auto thisupd = upgrades.begin(); thisupd != upgrades.end(); thisupd++) {
			j_st_obj_buf = json_object();
			j_st_buf = json_string(thisupd->first.c_str());
			json_object_set_new(j_st_obj_buf, "ip", j_st_buf);
			j_st_buf = json_string(thisupd->second->Message().c_str());
			json_object_set_new(j_st_obj_buf, "msg", j_st_buf);
			j_st_buf = json_integer(thisupd->second->UpgradePercent);
			json_object_set_new(j_st_obj_buf, "upd_percent", j_st_buf);
			j_st_buf = json_integer(thisupd->second->DownloadPercent);
			json_object_set_new(j_st_obj_buf, "dl_percent", j_st_buf);
			json_array_append_new(j_st_sts, j_st_obj_buf);
			upds_cnt++;
		}

		pthread_mutex_unlock(&map_lock);

		j_st_buf = json_integer(upds_cnt);
		json_object_set_new(out_data, "count", j_st_buf);
		json_object_set_new(out_data, "status", j_st_sts);

		return 0;

	} else if (op == "upgrade") {
		j_up_ips = json_object_get(in_data, "ips");
		if (!j_up_ips || !json_is_array(j_up_ips))
			return -2;

		j_fw_url = json_object_get(in_data, "fw_url");
		if (!j_fw_url || !json_is_string(j_fw_url))
			return -2;

		json_array_foreach(j_up_ips, i, j_up_thisip) {
			if (json_is_string(j_up_thisip)) {
				csbuf = json_string_value(j_up_thisip);
				amsd_mmupgrade_new(string(csbuf), json_string_value(j_fw_url));
				fprintf(stderr, "amsd: mmupgrade: Queueing %s\n", csbuf);
			}
		}

		return 0;
	} else if (op == "clear") {
		amsd_mmupgrade_clear();
		return 0;
	}

}
