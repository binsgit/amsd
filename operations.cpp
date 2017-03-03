/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "amsd.hpp"

std::map<std::string, void *> Operations;

void *amsd_operation_get(std::string name) {
	std::map<std::string, void *>::const_iterator target = Operations.find(name);

	if (target == Operations.end())
		return NULL;
	else
		return target->second;
}

bool amsd_operation_register(std::string name, int (*pfunc)(json_t*, json_t*&)) {
	if (Operations.insert(std::pair<std::string, void *>(name, (void *)pfunc)).second) {
		fprintf(stderr, "amsd: operations: Registered operation `%s' at %p\n", name.c_str(), pfunc);
		return true;
	} else {
		fprintf(stderr, "amsd: operations: Failed to registered operation `%s' at %p: operation exists\n",
			name.c_str(), pfunc);
		return false;
	}
}