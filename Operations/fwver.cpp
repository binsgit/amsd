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

#include "Operations.hpp"

int AMSD::Operations::fwver(json_t *in_data, json_t *&out_data){
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