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

std::map<std::string, std::pair<void *, bool>> OperationsList;

std::pair<void *, bool> AMSD::Operations::Get(std::string name) {
	std::map<std::string, std::pair<void *, bool>>::const_iterator target = OperationsList.find(name);

	if (target == OperationsList.end())
		throw 0x23336666;
	else
		return target->second;
}

bool AMSD::Operations::Register(std::string name, int (*pfunc)(json_t*, json_t*&), bool auth_required) {
	std::pair<void *, bool> thisopt = std::pair<void *, bool>((void *)pfunc, auth_required);

	if (OperationsList.insert(std::pair<std::string, std::pair<void *, bool>>(name, thisopt)).second) {
		fprintf(stderr, "amsd: Operations: Registered operation `%s' at %p\n", name.c_str(), pfunc);
		return true;
	} else {
		fprintf(stderr, "amsd: Operations: Failed to registered operation `%s' at %p: operation exists\n",
			name.c_str(), pfunc);
		return false;
	}
}

int AMSD::Operations::Init() {
	Register("glimpse", &glimpse, 0);
	Register("farmap", &farmap, 0);
	Register("user", &user);
	Register("history", &history, 0);
	Register("status", &status, 0);
	Register("issues", &issues, 0);
	Register("fwver", &fwver, 0);
	Register("ascset", &ascset);
	Register("supertac", &supertac);
	Register("controller", &controller);
	Register("mailreport", &mailreport);
	Register("config", &config);
	Register("rawapi", &rawapi);
	Register("login", &login, 0);
	Register("version", &version, 0);
	Register("poool", &poool, 0);
	Register("ctl_scanner", &ctl_scanner, 0);
	
	return 0;
}
