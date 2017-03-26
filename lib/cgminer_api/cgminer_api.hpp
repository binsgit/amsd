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

#ifndef AMSD_CGMINER_API_HPP
#define AMSD_CGMINER_API_HPP

#include "../../amsd.hpp"

using namespace std;

class CgMinerAPI {
public:
    //
    enum APIType {
	Summary = 1, EStats = 2, EDevs = 3, Pools = 4
    };

    // Static functions
    static const char *ToString(CgMinerAPI::APIType v);
    static int RequestRaw(Reimu::IPEndPoint ep, string in_data, string &out_data);


    // Constructor
    CgMinerAPI();

};


extern int cgminer_api_request_raw(string ip, uint16_t port, string in_data, string &out_data);


#endif //AMSD_CGMINER_API_HPP
