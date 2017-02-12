//
// Created by Reimu on 2017/2/4.
//

#include "../amsd.hpp"

int amsd_operation_fwver(json_t *in_data, json_t *&out_data){
	json_t *j_ip = json_object_get(in_data, "ip");
	json_t *j_fwver;
	string fwver;
	vector<uint8_t> buf;
	SSHConnection ssh;

	if (!j_ip)
		goto fatal;

	if (!json_is_string(j_ip))
		goto fatal;


	ssh.IP = string(json_string_value(j_ip));
	ssh.UserName = "root";
	ssh.Command = "cat /etc/avalon_version|head -n1";


	if (!ssh.Connect())
		fwver = "error: " + ssh.ErrorMessage.str();
	else {
		ssh.Execute();
		buf = ssh.GetReadBuffer();
		fwver = string((char *)&(buf[0]));
	}

	j_fwver = json_string(fwver.c_str());
	json_object_set_new(out_data, "fwver", j_fwver);

	return 0;

	fatal:
	return -1;
}