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

#ifndef AMSD_REPORT_HPP
#define AMSD_REPORT_HPP

#include "../amsd.hpp"

namespace Report {

    class Pool {
    public:
	string URL;
	string User;
	double GHS = 0;
    };

    class Controller {
    public:
	size_t Elapsed = 0;
	vector<uint8_t> Addr;
	uint16_t Port = 0;
    };

    class Farm {
    public:
	size_t Modules = 0;
	long double MHS = 0;

	vector<Controller> Controllers;
	map<pair<string, string>, Pool> Pools;
    };

    class Report {
    private:
	void CollectData();
    public:
	string Name;
	Farm Farm0;
	timeval ProcessTime;

	Report(string farm_name);

	string HTMLReport();

    };

}

#endif //AMSD_REPORT_HPP


