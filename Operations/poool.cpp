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

int AMSD::Operations::poool(json_t *in_data, json_t *&out_data){

	json_t *j_op = json_object_get(in_data, "op");
	json_t *j_nodes;
	json_t *j_poolcfg;
	json_t *j_addr;
	json_t *j_pool1url, *j_pool1user, *j_pool1pw, *j_pool2url, *j_pool2user, *j_pool2pw;


	string shell_cmd;


	if (!json_is_string(j_op))
		return -1;

	string op(json_string_value(j_op));

	if (op == "modify") {
		j_nodes = json_object_get(in_data, "nodes");
		j_poolcfg = json_object_get(in_data, "poolcfg");

		if (!json_is_array(j_nodes) || !json_is_object(j_poolcfg))
			return -1;

		j_pool1url = json_object_get(j_poolcfg, "pool1url");
		j_pool1user = json_object_get(j_poolcfg, "pool1user");
		j_pool1pw = json_object_get(j_poolcfg, "pool1pw");
		j_pool2url = json_object_get(j_poolcfg, "pool2url");
		j_pool2user = json_object_get(j_poolcfg, "pool2user");
		j_pool2pw = json_object_get(j_poolcfg, "pool2pw");

		shell_cmd += "#!/bin/sh\n";

		if (json_is_string(j_pool1url)) {
			shell_cmd += "uci set cgminer.default.pool1url='";
			shell_cmd += json_string_value(j_pool1url);
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool1user)) {
			shell_cmd += "uci set cgminer.default.pool1user='";
			shell_cmd += json_string_value(j_pool1user);
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool1pw)) {
			shell_cmd += "uci set cgminer.default.pool1pw='";
			shell_cmd += json_string_value(j_pool1pw);
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2url)) {
			shell_cmd += "uci set cgminer.default.pool2url='";
			shell_cmd += json_string_value(j_pool2url);
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2user)) {
			shell_cmd += "uci set cgminer.default.pool2user='";
			shell_cmd += json_string_value(j_pool2user);
			shell_cmd += "'\n";
		}

		if (json_is_string(j_pool2pw)) {
			shell_cmd += "uci set cgminer.default.pool2pw='";
			shell_cmd += json_string_value(j_pool2pw);
			shell_cmd += "'\n";
		}

		shell_cmd += "uci commit\n";

		size_t j;

		Lock_SuperRTACTasksList.lock();
		json_array_foreach(j_nodes, j, j_addr) {
			if (json_is_string(j_addr)) {
				SuperRTACSession *srtac = new SuperRTACSession;
				string s_ipbuf = json_string_value(j_addr);
				srtac->IP = s_ipbuf;
				srtac->ScriptName = "修改矿池信息";
				srtac->Script = shell_cmd;
				srtac->UUID = amsd_random_string();
				srtac->StartTime = time(NULL);

				srtac->Exec();
				SuperRTACTasksList.insert(pair<string, SuperRTACSession *>(s_ipbuf, srtac));
			}
		}
		Lock_SuperRTACTasksList.unlock();

	}

	return 0;
}